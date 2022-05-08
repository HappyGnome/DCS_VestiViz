net.log("VestiViz Hook called")



local base = _G

local lfs               = require('lfs')
local socket            = require("socket") 
local net               = require('net')
local DCS               = require("DCS") 
local Skin              = require('Skin')
local Gui               = require('dxgui')
local DialogLoader      = require('DialogLoader')
local Static            = require('Static')
local Tools             = require('tools')

package.cpath = package.cpath..";"..lfs.writedir().."Mods\\Services\\VestiViz\\bin\\?.dll;"

local VestiVizLib = nil

VestiViz = {
	-----------------------------------------------------
	config = {
		enabled = true,
		barWidth = 10,
		accFactor = 2,
		rotFactor = 1,
		somatogravFactor  = 1,
		maxw = 0.3,
		minw = 0.0,
		offlim = 0.4,
		halflife = 0.2,
		acclims = {x = 5,y = 20,z = 5, mY = 2},
		rotlims = {x = 0.5,y = 0.5,z = 0.5},
		logHotkey = "Ctrl+Alt+v",
		showHotkey = "Ctrl+Shift+v",
		incHotkey = "Ctrl+Shift+w",
		decHotkey = "Ctrl+Alt+w",
		colours = {"0xffdd00ee","0xffffff88","0x44ff4444","0x22ff2222","0x11ff1111","0x00ff0008"}
	},
	----------------------------------------------------
	hide = true,
	prev = {
		posCirc = {_v= {}, _at = 0, _maxSize = 8}, -- circular buffer
		time = 0,
		dt = 0,
		vel = {x=0,y=0,z=0},
		acc = {x=0,y=0,z=0},
		rot = {x=0,y=0,z=0},
		detectPeriod = {v = {0,0}, y = 0},
		bars = {
			left = {off = 0, w = 0},
			right = {off = 0, w = 0},
			top = {off = 0, w = 0},
			bottom = {off = 0, w = 0}
		}
	},
	minDt = 0.01,
	errorCooldown = 0,
	logModelUntil = 0,
	periodHint = 0, --Automatically adjust frames to step for velocity estimates to account for periodic noise
	accDescCrossEpoch = 0,
	start = true,
	colourInd = 1,
	everyNFrames = 2,
	logFile = io.open(lfs.writedir()..[[Logs\DCS-VestiViz-Overlay.log]], "w")
}
VestiViz.wlim = (VestiViz.config.maxw - VestiViz.config.minw)/2 
VestiViz.wcen = (VestiViz.config.maxw + VestiViz.config.minw)/2

-----------------------------------------------------------
-- CONFIG & UTILITY
VestiViz.log = function(str)
    if not str then 
        return
    end

    if VestiViz.logFile then
		local msg
		if type(str) == 'table' then
			msg = '{'
			for k,v in pairs(str) do
				local t = type(v)
				if t == 'string' or t == 'number' then
					msg = msg..k..':'..v..', '
				else
					msg = msg..k..':'..t..', '
				end
			end
			msg = msg..'}'
		else
			msg = str
		end
		VestiViz.logFile:write("["..os.date("%H:%M:%S").."] "..msg.."\r\n")
		VestiViz.logFile:flush()
    end
end

VestiViz.logCSV = function(str)
    if not str then 
        return
    end

    if VestiViz.logFile then
		local msg
		if type(str) == 'table' then
			msg = ''
			for k,v in pairs(str) do
				local t = type(v)
				if t == 'string' or t == 'number' then
					msg = msg..v..', '
				else
					msg = msg..t..', '
				end
			end
		else
			msg = str
		end
		VestiViz.logFile:write(msg .."\r\n")
		VestiViz.logFile:flush()
    end
end

function VestiViz.loadConfiguration()
    VestiViz.log("Config load starting")
	
    local cfg = Tools.safeDoFile(lfs.writedir()..'Config/VestiViz.lua', false)
	
    if (cfg and cfg.config) then
		for k,v in pairs(VestiViz.config) do
			if cfg.config[k] ~= nil then
				VestiViz.config[k] = cfg.config[k]
			end
		end        
    end
	
	VestiViz.saveConfiguration()
end

function VestiViz.saveConfiguration()
    U.saveInFile(VestiViz.config, 'config', lfs.writedir()..'Config/VestiViz.lua')
end
-----------------------------------------------------------
-- LOAD DLL
--VestiViz.log(_VERSION)
pcall(function()
	VestiViz._Lib = require('vestiviz')
	VestiViz.log("Loaded VestiViz.dll")
end)

if not VestiViz._Lib then
	VestiViz.log("Couldn't load VestiViz.dll")
end

-----------------------------------------------------------
--FILTERS AND ALGEBRA
VestiViz.addToTail = function(accum, u, decay)
	local d = 1-decay

	accum.x = d*u.x + decay * accum.x
	accum.y = d*u.y + decay * accum.y
	accum.z = d*u.z + decay * accum.z

end

VestiViz.compress = function(x,lim)
	return x / (lim + math.abs(x));
end

VestiViz.compcompress = function(u,lims)
	return {x = VestiViz.compress(u.x,lims.x),
			y = VestiViz.compress(u.y,lims.y),
			z = VestiViz.compress(u.z,lims.z)}
end

VestiViz.normalizeBar = function(bar)
	return {off = 0.5 + VestiViz.config.offlim * VestiViz.compress(bar.off, 1),
			w = VestiViz.wcen + VestiViz.wlim * VestiViz.compress(bar.w, 1)};
end

VestiViz.interpolateBar = function(bar1,bar2)
	return {off = 0.5* bar1.off + 0.5 * bar2.off,
			w = 0.5* bar1.w + 0.5 * bar2.w};
end

VestiViz.interpolateVec = function(u,v)
	return {x = 0.5* u.x + 0.5 * v.x,
			y = 0.5* u.y + 0.5 * v.y,
			z = 0.5* u.z + 0.5 * v.z};
end

VestiViz.compwiseMult = function(u,v)
	return {x = u.x * v.x,
			y = u.y * v.y,
			z = u.z * v.z};
end

VestiViz.scalarMult = function(u,a)
	return {x = u.x * a,
			y = u.y * a,
			z = u.z * a};
end

VestiViz.vecAdd = function(u,v)
	return {x = (v.x + u.x),
			y = (v.y + u.y),
			z = (v.z + u.z)};
end

VestiViz.lin = function(us)
	local ret = {x = 0,y = 0,z = 0};

	for k,v in pairs(us) do
		ret.x = ret.x + v[2] * v[1].x
		ret.y = ret.y + v[2] * v[1].y
		ret.z = ret.z + v[2] * v[1].z
	end
	return ret
end

VestiViz.vecDiff = function(u,v,dt)
	return {x = (v.x - u.x)/dt,
			y = (v.y - u.y)/dt,
			z = (v.z - u.z)/dt}
end

VestiViz.vecDot = function(u,v)
	return u.x*v.x + u.y*v.y + u.z*v.z
end

VestiViz.normalize = function(u)
	local r = u.x*u.x + u.y*u.y + u.z*u.z
	if r == 0 then
		return u
	end
	local a = 1/math.sqrt(r)
	return {x = u.x * a, y = u.y * a, z = u.z * a}
end
-- Best fit

--[[
	data = {{p = {x=...,y=...,z=...}, t = ...}, ...}
	output = {acc = {x=...,y=...,z=...}, x = {}, y = {}, z = {}} the estimated acceleration in the data
]]
VestiViz.bestFitAccel = function(data)
	local t1 = 0 -- mean time
	local t2 = 0
	local t3 = 0
	local t4 = 0 --mean of (time - t1)^4
	local N = 0
	local p1 = {x=0,y=0,z=0} -- mean of p
	local pt1 = {x=0,y=0,z=0} -- mean of p*t
	local pt2 = {x=0,y=0,z=0}
	local ret = {--[[x = {x=0,y=0,z=0}, y = {x=0,y=0,z=0},z = {x=0,y=0,z=0}]]}

	for k,v in pairs(data) do
		N = N + 1
		t1 = t1 + v.t
		p1 = VestiViz.vecAdd(p1,v.p)
		--[[ret.x = VestiViz.vecAdd(ret.x,v.x)
		ret.y = VestiViz.vecAdd(ret.y,v.y)
		ret.z = VestiViz.vecAdd(ret.z,v.z)]]
	end
	if N == 0 then
		return nil
	end
	local nInv = 1/N
	t1 = t1 * nInv
	p1 = VestiViz.scalarMult(p1,nInv)
	--[[ret.x = VestiViz.scalarMult(ret.x,nInv)
	ret.y = VestiViz.scalarMult(ret.y,nInv)
	ret.z = VestiViz.scalarMult(ret.z,nInv)]]

	for k,v in pairs(data) do
		local tau = v.t
		local tau2 = tau * v.t
		local tau3 = tau2 * v.t
		local tau4 = tau3 * v.t

		t2 = t2 + tau2
		t3 = t3 + tau3
		t4 = t4 + tau4

		pt1 = VestiViz.vecAdd(pt1,VestiViz.scalarMult(v.p,tau))	
		pt2 = VestiViz.vecAdd(pt2,VestiViz.scalarMult(v.p,tau2))	
	end
	pt1 = VestiViz.scalarMult(pt1,nInv)
	pt2 = VestiViz.scalarMult(pt2,nInv)

	local D = t2*t4 -t2*t2*t2 - t3*t3
	if D == 0 then
		return nil
	end

	ret.acc = VestiViz.lin({{pt2,2*t2/D},{p1,-2*t2*t2/D},{pt1,-2*t3/D}}) -- 2 factor b/c acceleration is twice the quadratic coeff
	return ret
end

--Circular buffers
VestiViz.getCirc = function(circ,offset)
	return circ._v[1 + (circ._at + offset - 1)%(#circ._v)]
end

VestiViz.setNextCirc = function(circ,val)
	circ._at = 1 + (circ._at % circ._maxSize)
	circ._v[circ._at] = val
end

-----------------------------------------------------------
VestiViz.setItemPicCol = function (item,col)
	local skin = item:getSkin()
	skin.skinData.states.released[1].picture.color = col
	item:setSkin(skin)
end

VestiViz.setImageBaseDir = function (item,dir)
	local skin = item:getSkin()
	skin.skinData.states.released[1].picture.file = dir .. skin.skinData.states.released[1].picture.file
	item:setSkin(skin)
end

VestiViz.LoadDlg = function()
	VestiViz.log("Creating VestiViz Overlay")
	if VestiViz.window ~= nil then
		VestiViz.window:setVisible(false)
	end

	VestiViz.window = DialogLoader.spawnDialogFromFile(lfs.writedir() .. 'Mods\\Services\\VestiViz\\UI\\Overlay.dlg', cdata)

	VestiViz.setImageBaseDir(VestiViz.window.LeftArrow,lfs.writedir())
	VestiViz.setImageBaseDir(VestiViz.window.RightArrow,lfs.writedir())
	VestiViz.setImageBaseDir(VestiViz.window.TopArrow,lfs.writedir())
	VestiViz.setImageBaseDir(VestiViz.window.BottomArrow,lfs.writedir())
	
	VestiViz.window:setVisible(true)
	VestiViz.width, VestiViz.height = Gui.GetWindowSize()
	VestiViz.window:setSize(VestiViz.width, VestiViz.height)
	VestiViz.UpdateShowHideDlg()
	
	VestiViz.window:addHotKeyCallback(VestiViz.config.logHotkey, VestiViz.onLogHotkey)
	VestiViz.window:addHotKeyCallback(VestiViz.config.showHotkey, VestiViz.onShowHotkey)
	VestiViz.window:addHotKeyCallback(VestiViz.config.decHotkey, VestiViz.onDecHotkey)
	VestiViz.window:addHotKeyCallback(VestiViz.config.incHotkey, VestiViz.onIncHotkey)

	VestiViz.log("VestiViz Overlay created")

end

VestiViz.UpdateShowHideDlg = function()
	VestiViz.window.LeftArrow:setVisible(not VestiViz.hide)
	VestiViz.window.RightArrow:setVisible(not VestiViz.hide)
	VestiViz.window.TopArrow:setVisible(not VestiViz.hide)
	VestiViz.window.BottomArrow:setVisible(not VestiViz.hide)
end

VestiViz.DetectPeriodicNoise = function (yNow, dt)
	local crudeYVel = (yNow - VestiViz.prev.detectPeriod.y)/dt
	--Detect periodic noise
	if crudeYVel - VestiViz.prev.detectPeriod.v[2] < 0 and VestiViz.prev.detectPeriod.v[2]  - VestiViz.prev.detectPeriod.v[1] >= 0 then -- desc crossing
		if VestiViz.accDescCrossEpoch > 0 then
			VestiViz.periodHint = VestiViz.accDescCrossEpoch - 1
		end
		VestiViz.accDescCrossEpoch = 1
	elseif VestiViz.accDescCrossEpoch > 0 then
		VestiViz.accDescCrossEpoch = (1+VestiViz.accDescCrossEpoch)%VestiViz.prev.posCirc._maxSize
		if VestiViz.accDescCrossEpoch == 0 then
			VestiViz.periodHint = 0
		end
	end
	VestiViz.prev.detectPeriod.y = yNow
	VestiViz.prev.detectPeriod.v[1] = VestiViz.prev.detectPeriod.v[2]
	VestiViz.prev.detectPeriod.v[2] = crudeYVel
end

VestiViz.frames = 0
VestiViz.doOnSimFrame = function()
	
	if VestiViz._Pipeline == nil then return end

	local now = base.Export.LoGetModelTime() --getModelTime -- socket.gettime()/1000 -- DCS.getRealTime()
	local pos3 = base.Export.LoGetCameraPosition()

	VestiViz._Pipeline.addDatum(VestiViz._PipelineData.inputp,now, pos3);
	VestiViz._Pipeline.addDatum(VestiViz._PipelineData.inputxy,now, pos3);
	VestiViz._Pipeline.addDatum(VestiViz._PipelineData.inputframe1,now, pos3);
	VestiViz._Pipeline.addDatum(VestiViz._PipelineData.inputframe2,now, pos3);

	local datum = VestiViz._Pipeline.getDatum(VestiViz._PipelineData.output);


	--VestiViz.window.DebugData:setText("Hi:"..bottom.off)
	VestiViz.window.BottomArrow:setBounds(
		VestiViz.width * (datum.off.bottom - datum.w.bottom), 
		VestiViz.height - VestiViz.config.barWidth, 
		2 * VestiViz.width * datum.w.bottom,
		VestiViz.config.barWidth)
		
	VestiViz.window.TopArrow:setBounds(
		VestiViz.width * (datum.off.top - datum.w.top), 
		0, 
		2 * VestiViz.width * datum.w.top,  
		VestiViz.config.barWidth)	
		
	VestiViz.window.LeftArrow:setBounds(
		0, 
		VestiViz.height * (datum.off.left - datum.w.left), 
		VestiViz.config.barWidth,  
		2 * VestiViz.height * datum.w.left)
		
	VestiViz.window.RightArrow:setBounds(
		VestiViz.width-VestiViz.config.barWidth, 
		VestiViz.height * (datum.off.right - datum.w.right), 
		VestiViz.config.barWidth,  
		2 * VestiViz.height * datum.w.right)

	--[[VestiViz.frames = VestiViz.frames + 1
	if VestiViz.window then
		local now = base.Export.LoGetModelTime() --getModelTime -- socket.gettime()/1000 -- DCS.getRealTime()
		local pos3 = base.Export.LoGetCameraPosition()
		local here = pos3.p
		local X = pos3.x
		local Y = pos3.y
		local Z = pos3.z
		local vel = VestiViz.prev.vel
	
		if VestiViz.start then
			VestiViz.prev.time = now
			VestiViz.prev.vel = vel
			VestiViz.prev.posCirc = {_v= {[1] = {p = here, t= now, x = {x=0,y=0,z=0}, y= {x=0,y=0,z=0},z = {x=0,y=0,z=0}}}, _at = 1, _maxSize = 8}
			VestiViz.start = false
			VestiViz.prev.dt = 0.01
			VestiViz.periodHint = 0
		else
			local dt = now - VestiViz.prev.time;
		
			if dt > VestiViz.minDt and VestiViz.frames >= VestiViz.everyNFrames then
				local prev = VestiViz.getCirc(VestiViz.prev.posCirc,0)
				VestiViz.setNextCirc(VestiViz.prev.posCirc,{p = here, t = now, x = X, y = Y, z = Z})

				local worldAcc = VestiViz.bestFitAccel(VestiViz.prev.posCirc._v).acc--
				worldAcc.y = worldAcc.y + 9.81
				
				local decayFactor = math.pow(0.5,dt/VestiViz.config.halflife)

				local rawYAcc = VestiViz.vecDot(worldAcc,Y) - 9.81

				if rawYAcc < 0 then
					rawYAcc = rawYAcc * VestiViz.config.acclims.mY
				end
				
				VestiViz.addToTail(VestiViz.prev.acc,
									VestiViz.compcompress(
										{
											x = VestiViz.vecDot(worldAcc,X),
									 		y = rawYAcc,
									 		z = VestiViz.vecDot(worldAcc,Z)
										},
										VestiViz.config.acclims),
									decayFactor
								)
								 
				local viewAcc = VestiViz.scalarMult(VestiViz.prev.acc,VestiViz.config.accFactor)
				local somatoGrav = VestiViz.scalarMult(VestiViz.prev.acc,VestiViz.config.somatogravFactor)

				local dX = VestiViz.vecDiff(prev.x,X,dt)

				local dY = VestiViz.vecDiff(prev.y,Y,dt)
				
				VestiViz.addToTail(VestiViz.prev.rot,
									VestiViz.compcompress(
										{
											x = VestiViz.vecDot(Z,dY),
											y = -VestiViz.vecDot(Z,dX),
											z = VestiViz.vecDot(Y,dX)
										},
										VestiViz.config.rotlims),
									decayFactor
								)
								
				local viewRot = VestiViz.scalarMult(VestiViz.prev.rot,VestiViz.config.rotFactor)
					
				local bars = {
					bottom = VestiViz.normalizeBar({
						off = viewRot.x + viewRot.y - somatoGrav.z,
						w = viewAcc.y - viewAcc.x
					}),							  
					top = VestiViz.normalizeBar({
						off = -viewRot.x + viewRot.y + somatoGrav.z,
						w = -viewAcc.y - viewAcc.x
					}),							  
					left = VestiViz.normalizeBar({
						off = viewRot.x + viewRot.z + (somatoGrav.x - somatoGrav.z),
						w = viewAcc.z - viewAcc.x
					}),					
					right = VestiViz.normalizeBar({
						off = - viewRot.x + viewRot.z + (somatoGrav.z + somatoGrav.x),
						w = - viewAcc.z - viewAcc.x
					})
				}
				local bottom = VestiViz.interpolateBar(bars.bottom, VestiViz.prev.bars.bottom)
						  
				local top = VestiViz.interpolateBar(bars.top, VestiViz.prev.bars.top)
						  
				local left = VestiViz.interpolateBar(bars.left, VestiViz.prev.bars.left)
				
				local right = VestiViz.interpolateBar(bars.right, VestiViz.prev.bars.right)

				VestiViz.prev.bars = bars
				
				--VestiViz.window.DebugData:setText("Hi:"..bottom.off)
				VestiViz.window.BottomArrow:setBounds(
					VestiViz.width * (bottom.off - bottom.w), 
					VestiViz.height - VestiViz.config.barWidth, 
					2 * VestiViz.width * bottom.w,
					VestiViz.config.barWidth)
					
				VestiViz.window.TopArrow:setBounds(
					VestiViz.width * (top.off - top.w), 
					0, 
					2 * VestiViz.width * top.w,  
					VestiViz.config.barWidth)	
					
				VestiViz.window.LeftArrow:setBounds(
					0, 
					VestiViz.height * (left.off - left.w), 
					VestiViz.config.barWidth,  
					2 * VestiViz.height * left.w)
					
				VestiViz.window.RightArrow:setBounds(
					VestiViz.width-VestiViz.config.barWidth, 
					VestiViz.height * (right.off - right.w), 
					VestiViz.config.barWidth,  
					2 * VestiViz.height * right.w)
					
				VestiViz.prev.time = now
				VestiViz.prev.vel = vel
				VestiViz.prev.dt = dt
				
				
				if VestiViz.logModelUntil > now then
					VestiViz.logCSV({dt, worldAcc.x, worldAcc.y, worldAcc.z, here.x,
					here.y,here.z, vel.x,vel.y,vel.z, VestiViz.frames, 
					VestiViz.prev.acc.x, VestiViz.prev.acc.y, VestiViz.prev.acc.z, VestiViz.accDescCrossEpoch,
					VestiViz.periodHint, VestiViz.prev.posCirc._at})
				end
				VestiViz.frames = 0
			end
		end

	end--]]
end

--------------------------------------------------------------
VestiViz.initPipeline = function()
	if VestiViz._Lib == nil then return end

	VestiViz._Pipeline = VestiViz._Lib.newPipeline();

	local frameinput1, frameinput2;
	local leaf1, input1 = VestiViz._Pipeline.accelByRegressionFilterPoint();
	leaf1 = VestiViz._Pipeline.staticAddFilterPoint({x = 0, y = 9.81, z = 0},leaf1);
	leaf1, frameinput1 = VestiViz._Pipeline.dynMatMultFilterPoint(leaf1,nil);
	print(leaf1..":"..frameinput1);
	leaf1 = VestiViz._Pipeline.staticAddFilterPoint({x = 0, y = -9.81, z = 0},leaf1);
	leaf1 = VestiViz._Pipeline.quickCompressFilterPoint({x = 1, y = 1, z = 1},leaf1);
	leaf1 = VestiViz._Pipeline.expDecayFilterPoint(100.0,leaf1);
	leaf1 = VestiViz._Pipeline.matMultFilterPointToWOff({
					0.5, -0.5, 0.0,--T width
					0.5, 0.0, -0.5,--R width
					0.5, 0.5, 0.0,--B width
					0.5, 0.0, 0.5,--L width
					0.0, 0.0, 1.0, --T somatograv
					1.0, 0.0, 1.0,--R
					0.0, 0.0, -1.0,--B
					1.0, 0.0, -1.0},
					leaf1);
	local leaf2, input2 = VestiViz._Pipeline.simpleDiffFilterXY();
	leaf2, frameinput2 = VestiViz._Pipeline.dynMatMultPickFilterXYtoPoint({
					{2,1}, --x-axis rot
					{2,0}, --negative y-axis rot
					{1,0}} --z-axis rot
					,leaf2);
	leaf2 = VestiViz._Pipeline.quickCompressFilterPoint({x = 1, y = 1, z = 1},leaf2);
	leaf2 = VestiViz._Pipeline.expDecayFilterPoint(100.0,leaf2);
	leaf2 = VestiViz._Pipeline.matMultFilterPointToWOff(
					{0.0, 0.0, 0.0,--T width
					0.0, 0.0, 0.0,--R
					0.0, 0.0, 0.0,--B
					0.0, 0.0, 0.0,--L 
					-1.0, 1.0, 0.0, --T displacement
					-1.0, 0.0, 1.0,--R
					1.0, 1.0, 0.0,--B
					1.0, 0.0, 1.0},
					leaf2);
	local leaf3 = VestiViz._Pipeline.linCombFilterWOff(0.5,0.5,leaf1,leaf2);
	leaf3 = VestiViz._Pipeline.quickCompressFilterWOff({w = {top = 1,right = 1,bottom = 1,left = 1}, off = {top = 1,right = 1,bottom = 1,left = 1}},leaf3);
	leaf3 = VestiViz._Pipeline.convolveOutputFilterWOff({0.25,0.5,0.25},leaf3,3);
	local output = VestiViz._Pipeline.makeWOffOutput(leaf3);

	VestiViz._PipelineData = {inputp = input1,
							  inputxy = input2,
							  inputframe1 = frameinput1,
							  inputframe2 = frameinput2,
							  outputWOff = output}

	local error = VestiViz._Pipeline.popError();
	while error ~= nil do
		VestiViz.log(error);
		error = VestiViz._Pipeline.popError();
	end
end
 
--------------------------------------------------------------
-- CALLBACKS
VestiViz.onSimulationFrame = function()	
	if not VestiViz.window then
		VestiViz.loadConfiguration()
		VestiViz.LoadDlg()
	end

	if VestiViz.hide == false then
			VestiViz.errorCooldown = math.max(VestiViz.errorCooldown - 1,0)
		local status,err = pcall(VestiViz.doOnSimFrame)
		if not status and VestiViz.errorCooldown <= 0 then
			VestiViz.log(err)
			VestiViz.errorCooldown = 4000
		end
	end
end

VestiViz.onSimulationStart = function()
	VestiViz.start = true
	VestiViz.loadConfiguration()
	VestiViz.LoadDlg()
	VestiViz.initPipeline();
	if VestiViz._Pipeline ~= nil then VestiViz._Pipeline.start() end
end

VestiViz.onSimulationStop = function()
	VestiViz.hide = true
	VestiViz.UpdateShowHideDlg();
	if VestiViz._Pipeline ~= nil then VestiViz._Pipeline.stop() end
end
--------------------------------------------------------------
-- HOTKEYS
VestiViz.onLogHotkey = function()
	VestiViz.logModelUntil = base.Export.LoGetModelTime() + 3

	VestiViz.logCSV({'dt', 'accx', 'accy', 'accz', 'x',
					'y','z', 'vx','vy','vz', 'frames', 
					'vax', 'vay', 'vaz', 'epoch',
					'periodHint', 'circAt'})
end

VestiViz.onShowHotkey = function()
	VestiViz.LoadDlg() -- debug
	VestiViz.hide =  (not VestiViz.hide) or (not VestiViz.config.enabled)
	VestiViz.UpdateShowHideDlg()
end

VestiViz.onDecHotkey = function()
	VestiViz.colourInd = math.max(VestiViz.colourInd-1,1)
	local colour = VestiViz.config.colours[VestiViz.colourInd]
	VestiViz.setItemPicCol(VestiViz.window.LeftArrow,colour)
	VestiViz.setItemPicCol(VestiViz.window.RightArrow,colour)
	VestiViz.setItemPicCol(VestiViz.window.TopArrow,colour)
	VestiViz.setItemPicCol(VestiViz.window.BottomArrow,colour)
end

VestiViz.onIncHotkey = function()
	VestiViz.colourInd = math.min(1 + VestiViz.colourInd,#VestiViz.config.colours)
	local colour = VestiViz.config.colours[VestiViz.colourInd]
	VestiViz.setItemPicCol(VestiViz.window.LeftArrow,colour)
	VestiViz.setItemPicCol(VestiViz.window.RightArrow,colour)
	VestiViz.setItemPicCol(VestiViz.window.TopArrow,colour)
	VestiViz.setItemPicCol(VestiViz.window.BottomArrow,colour)

end
--------------------------------------------------------------
DCS.setUserCallbacks(VestiViz)