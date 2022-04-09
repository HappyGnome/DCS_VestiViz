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


vestiViz = {
	enabled = true,
	-----------------------------------------------------
	barWidth = 10,
	accFactor = 2,
	rotFactor = 1,
	somatogravFactor  = 1,
	maxw = 0.25,
	minw = 0.005,
	offlim = 0.4,
	----------------------------------------------------
	acclims = {x = 0.5,y = 10,z = 0.5, mY = 2},
	rotlims = {x = 2,y = 2,z = 2},
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
	halflife = 0.5,
	minDt = 0.01,
	errorCooldown = 0,
	logModelUntil = 0,
	periodHint = 0, --Automatically adjust frames to step for velocity estimates to account for periodic noise
	accDescCrossEpoch = 0,
	start = true,
	logFile = io.open(lfs.writedir()..[[Logs\DCS-VestiViz-Overlay.log]], "w")
}
vestiViz.wlim = (vestiViz.maxw - vestiViz.minw)/2 
vestiViz.wcen = (vestiViz.maxw + vestiViz.minw)/2

-----------------------------------------------------------
--FILTERS AND ALGEBRA
vestiViz.addToTail = function(accum, u, decay)
	local d = 1-decay

	accum.x = d*u.x + decay * accum.x
	accum.y = d*u.y + decay * accum.y
	accum.z = d*u.z + decay * accum.z

end

vestiViz.compress = function(x,lim)
	return x / (lim + math.abs(x));
end

vestiViz.compcompress = function(u,lims)
	return {x = vestiViz.compress(u.x,lims.x),
			y = vestiViz.compress(u.y,lims.y),
			z = vestiViz.compress(u.z,lims.z)}
end

vestiViz.normalizeBar = function(bar)
	return {off = 0.5 + vestiViz.offlim * vestiViz.compress(bar.off, 1),
			w = vestiViz.wcen + vestiViz.wlim * vestiViz.compress(bar.w, 1)};
end

vestiViz.interpolateBar = function(bar1,bar2)
	return {off = 0.5* bar1.off + 0.5 * bar2.off,
			w = 0.5* bar1.w + 0.5 * bar2.w};
end

vestiViz.compwiseMult = function(u,v)
	return {x = u.x * v.x,
			y = u.y * v.y,
			z = u.z * v.z};
end

vestiViz.scalarMult = function(u,a)
	return {x = u.x * a,
			y = u.y * a,
			z = u.z * a};
end

vestiViz.vecDiff = function(u,v,dt)
	return {x = (v.x - u.x)/dt,
			y = (v.y - u.y)/dt,
			z = (v.z - u.z)/dt}
end

vestiViz.vecDot = function(u,v)
	return u.x*v.x + u.y*v.y + u.z*v.z
end

vestiViz.normalize = function(u)
	local r = u.x*u.x + u.y*u.y + u.z*u.z
	if r < 1 then
		return u
	end
	local a = 1/math.sqrt(r)
	return {x = u.x * a, y = u.y * a, z = u.z * a}
end

--Circular buffers
vestiViz.getCirc = function(circ,offset)
	return circ._v[1 + (circ._at + offset - 1)%(#circ._v)]
end

vestiViz.setNextCirc = function(circ,val)
	circ._at = 1 + (circ._at % circ._maxSize)
	circ._v[circ._at] = val
end
-----------------------------------------------------------
-- CONFIG & UTILITY
vestiViz.log = function(str)
    if not str then 
        return
    end

    if vestiViz.logFile then
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
		vestiViz.logFile:write("["..os.date("%H:%M:%S").."] "..msg.."\r\n")
		vestiViz.logFile:flush()
    end
end

function vestiViz.loadConfiguration()
    vestiViz.log("Config load starting")
	
	vestiViz.config = {
		logHotkey = "Ctrl+Shift+q"
	}
	
    local cfg = Tools.safeDoFile(lfs.writedir()..'Config/VestiViz.lua', false)
	
    if (cfg and cfg.config) then
		for k,v in pairs(vestiViz.config) do
			if cfg.config[k] ~= nil then
				vestiViz.config[k] = cfg.config[k]
			end
		end        
    end
	
	vestiViz.saveConfiguration()
end

function vestiViz.saveConfiguration()
    U.saveInFile(vestiViz.config, 'config', lfs.writedir()..'Config/VestiViz.lua')
end

-----------------------------------------------------------
vestiViz.LoadDlg = function()
	vestiViz.log("Creating VestiViz Overlay")
	vestiViz.window = DialogLoader.spawnDialogFromFile(lfs.writedir() .. 'Mods\\Services\\VestiViz\\UI\\Overlay.dlg', cdata)
	vestiViz.window:setVisible(true)
	vestiViz.width, vestiViz.height = Gui.GetWindowSize()
	vestiViz.window:setSize(vestiViz.width, vestiViz.height)
	
	vestiViz.window:addHotKeyCallback(vestiViz.config.logHotkey, vestiViz.onLogHotkey)

	vestiViz.log("VestiViz Overlay created")

end

vestiViz.DetectPeriodicNoise = function (yNow, dt)
	local crudeYVel = (yNow - vestiViz.prev.detectPeriod.y)/dt
	--Detect periodic noise
	if crudeYVel - vestiViz.prev.detectPeriod.v[2] < 0 and vestiViz.prev.detectPeriod.v[2]  - vestiViz.prev.detectPeriod.v[1] >= 0 then -- desc crossing
		if vestiViz.accDescCrossEpoch > 0 then
			vestiViz.periodHint = vestiViz.accDescCrossEpoch - 1
		end
		vestiViz.accDescCrossEpoch = 1
	elseif vestiViz.accDescCrossEpoch > 0 then
		vestiViz.accDescCrossEpoch = (1+vestiViz.accDescCrossEpoch)%vestiViz.prev.posCirc._maxSize
		if vestiViz.accDescCrossEpoch == 0 then
			vestiViz.periodHint = 0
		end
	end
	vestiViz.prev.detectPeriod.y = yNow
	vestiViz.prev.detectPeriod.v[1] = vestiViz.prev.detectPeriod.v[2]
	vestiViz.prev.detectPeriod.v[2] = crudeYVel
end

vestiViz.frames = 0
vestiViz.doOnSimFrame = function()
	
	if vestiViz.config == nil then
        vestiViz.loadConfiguration()
    end
	
	if not vestiViz.window then
		vestiViz.LoadDlg()
	end
	
	vestiViz.frames = vestiViz.frames + 1
	if vestiViz.window then
		local now = base.Export.LoGetModelTime() --getModelTime -- socket.gettime()/1000 -- DCS.getRealTime()
		local pos3 = base.Export.LoGetCameraPosition()
		local here = pos3.p
		local X = pos3.x
		local Y = pos3.y
		local vel = vestiViz.prev.vel
	
		if vestiViz.start then
			vestiViz.prev.time = now
			vestiViz.prev.vel = vel
			vestiViz.prev.posCirc = {_v= {[1] = {p = here, t= now, x = {x=0,y=0,z=0}, y= {x=0,y=0,z=0}}}, _at = 1, _maxSize = 8}
			vestiViz.start = false
			vestiViz.prev.dt = 0.01
			vestiViz.periodHint = 0
		else
			local dt = now - vestiViz.prev.time;
		
			if dt > vestiViz.minDt then

				vestiViz.DetectPeriodicNoise(here.y,dt)
				
				local prev = vestiViz.getCirc(vestiViz.prev.posCirc, -vestiViz.periodHint)
				
				local dtLong = now - prev.t -- time since previous sample to use for velocity derivatives
				vel = vestiViz.vecDiff(prev.p,here,dtLong)
				
				local worldAcc = vestiViz.vecDiff(vestiViz.prev.vel,vel,(dt + vestiViz.prev.dt)/2)
				
				local decayFactor = math.pow(0.5,dt/vestiViz.halflife)

				local rawYAcc = vestiViz.vecDot({ x = worldAcc.x, y = worldAcc.y + 9.81, z = worldAcc.z},Y) - 9.81

				if rawYAcc < 0 then
					rawYAcc = rawYAcc * vestiViz.acclims.mY
				end
				
				vestiViz.addToTail(vestiViz.prev.acc,
									vestiViz.compcompress(
										{
											x = vestiViz.vecDot(worldAcc,X),
									 		y = rawYAcc,
									 		z = vestiViz.vecDot(worldAcc,pos3.z)
										},
										vestiViz.acclims),
									decayFactor
								)
								 
				local viewAcc = vestiViz.scalarMult(vestiViz.prev.acc,vestiViz.accFactor)
				local somatoGrav = vestiViz.scalarMult(vestiViz.prev.acc,vestiViz.somatogravFactor)

				local dX = vestiViz.vecDiff(prev.x,X,dtLong)

				local dY = vestiViz.vecDiff(prev.y,Y,dtLong)
				
				vestiViz.addToTail(vestiViz.prev.rot,
									vestiViz.compcompress(
										{
											x = vestiViz.vecDot(pos3.z,dY),
											y = -vestiViz.vecDot(pos3.z,dX),
											z = vestiViz.vecDot(Y,dX)
										},
										vestiViz.rotlims),
									decayFactor
								)
								
				local viewRot = vestiViz.scalarMult(vestiViz.prev.rot,vestiViz.rotFactor)
					
				local bars = {
					bottom = vestiViz.normalizeBar({
						off = viewRot.x + viewRot.y - somatoGrav.z,
						w = viewAcc.y - viewAcc.x
					}),							  
					top = vestiViz.normalizeBar({
						off = -viewRot.x + viewRot.y + somatoGrav.z,
						w = -viewAcc.y - viewAcc.x
					}),							  
					left = vestiViz.normalizeBar({
						off =  viewRot.x + viewRot.z + (somatoGrav.x - somatoGrav.z),
						w = viewAcc.z - viewAcc.x
					}),					
					right = vestiViz.normalizeBar({
						off = - viewRot.x + viewRot.z + (somatoGrav.z + somatoGrav.x),
						w = - viewAcc.z - viewAcc.x
					})
				}
				local bottom = vestiViz.interpolateBar(bars.bottom, vestiViz.prev.bars.bottom)
						  
				local top = vestiViz.interpolateBar(bars.top, vestiViz.prev.bars.top)
						  
				local left = vestiViz.interpolateBar(bars.left, vestiViz.prev.bars.left)
				
				local right = vestiViz.interpolateBar(bars.right, vestiViz.prev.bars.right)

				vestiViz.prev.bars = bars
				
				--vestiViz.window.DebugData:setText("Hi:"..bottom.off)
				vestiViz.window.BottomArrow:setBounds(
					vestiViz.width * (bottom.off - bottom.w), 
					vestiViz.height - vestiViz.barWidth, 
					2 * vestiViz.width * bottom.w,
					vestiViz.barWidth)
					
				vestiViz.window.TopArrow:setBounds(
					vestiViz.width * (top.off - top.w), 
					0, 
					2 * vestiViz.width * top.w,  
					vestiViz.barWidth)	
					
				vestiViz.window.LeftArrow:setBounds(
					0, 
					vestiViz.height * (left.off - left.w), 
					vestiViz.barWidth,  
					2 * vestiViz.height * left.w)
					
				vestiViz.window.RightArrow:setBounds(
					vestiViz.width-vestiViz.barWidth, 
					vestiViz.height * (right.off - right.w), 
					vestiViz.barWidth,  
					2 * vestiViz.height * right.w)
					
				vestiViz.prev.time = now
				vestiViz.prev.vel = vel
				vestiViz.prev.dt = dt
				vestiViz.setNextCirc(vestiViz.prev.posCirc,{p = here, t = now, x = X, y = Y})
				
				if vestiViz.logModelUntil > now then
					vestiViz.log({dt = dt, accx = worldAcc.x, accy = worldAcc.y, accz = worldAcc.z, x= here.x,
					y=here.y,z=here.z, vx=vel.x,vy=vel.y,vz=vel.z, frames = vestiViz.frames, 
					vax=vestiViz.prev.acc.x, vay=vestiViz.prev.acc.y, vaz=vestiViz.prev.acc.z, epoch = vestiViz.accDescCrossEpoch,
					periodHint = vestiViz.periodHint, circAt = vestiViz.prev.posCirc._at})
				end
				vestiViz.frames = 0
			end
		end

	end
end
 
--------------------------------------------------------------
-- CALLBACKS
vestiViz.onSimulationFrame = function()	
	if vestiViz.enabled == true then
			vestiViz.errorCooldown = math.max(vestiViz.errorCooldown - 1,0)
		local status,err = pcall(vestiViz.doOnSimFrame)
		if not status and vestiViz.errorCooldown <= 0 then
			vestiViz.log(err)
			vestiViz.errorCooldown = 4000
		end
	end	
end 

vestiViz.onSimulationStart = function()
	vestiViz.start = true
end
--------------------------------------------------------------
-- HOTKEYS
vestiViz.onLogHotkey = function()
	vestiViz.logModelUntil = base.Export.LoGetModelTime() + 3
end
--------------------------------------------------------------
DCS.setUserCallbacks(vestiViz)