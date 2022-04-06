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
	accFactors  = {x = 0.005,y = 0.005,z = 0.005, mY = 2},
	rotFactors = {x = 0.2, y = 0.2, z = 0.2},
	somatogravFactor  = 0.25,
	maxw = 0.25,
	minw = 0.005,
	offlim = 0.4,
	----------------------------------------------------
	acclim = 100,
	prevPos = {x=0,y=0,z=0},
	prevTime = 0,
	prevVel = {x=0,y=0,z=0},
	acc = {x=0,y=0,z=0},
	rot = {x=0,y=0,z=0},
	halflife = 0.8,
	prevX = {x=0,y=0,z=0},
	prevY = {x=0,y=0,z=0},
	errorCooldown = 0,
	start = true,
	logFile = io.open(lfs.writedir()..[[Logs\DCS-VestiViz-Overlay.log]], "w")
}
vestiViz.wlim = (vestiViz.maxw - vestiViz.minw)/2 
vestiViz.wcen = (vestiViz.maxw + vestiViz.minw)/2

vestiViz.addToTail = function(accum, u, decay)
	local d = 1-decay

	accum.x = d*u.x + decay * accum.x
	accum.y = d*u.y + decay * accum.y
	accum.z = d*u.z + decay * accum.z

end

vestiViz.compress = function(x,lim,cen)
	return cen + lim * x / (1 + math.abs(x));
end

vestiViz.normalizeBar = function(bar)
	return {off = vestiViz.compress(bar.off, vestiViz.offlim,0),
			w = vestiViz.compress(bar.w, vestiViz.wlim,vestiViz.wcen)};
end

vestiViz.compwiseMult = function(u,v)
	return {x = u.x * v.x,
			y = u.y * v.y,
			z = u.z * v.z};
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

vestiViz.log = function(str)
    if not str then 
        return
    end

    if vestiViz.logFile then
        vestiViz.logFile:write("["..os.date("%H:%M:%S").."] "..str.."\r\n")
        vestiViz.logFile:flush()
    end
end

vestiViz.LoadDlg = function()
	vestiViz.log("Creating VestiViz Overlay")
	vestiViz.window = DialogLoader.spawnDialogFromFile(lfs.writedir() .. 'Mods\\Services\\VestiViz\\UI\\Overlay.dlg', cdata)
	vestiViz.window:setVisible(true)
	vestiViz.width, vestiViz.height = Gui.GetWindowSize()
	vestiViz.window:setSize(vestiViz.width, vestiViz.height)
	
	--vestiViz.window.BottomArrow.skin.states.released[1].picture.file = lfs.writedir() .. "Mods\\Services\\VestiViz\\Skins\\Arrow.png"]]
	vestiViz.log("VestiViz Overlay created")

end
--Done = false
 
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

vestiViz.doOnSimFrame = function()

	if not vestiViz.window then
		vestiViz.LoadDlg()
	end

	if vestiViz.window then
		local now = base.Export.LoGetModelTime()
		local pos3 = base.Export.LoGetCameraPosition()
		local here = pos3.p
		local X = pos3.x
		local Y = pos3.y
		local vel = vestiViz.prevVel
	
		if vestiViz.start then
			vestiViz.prevTime = now
			vestiViz.prevVel = vel
			vestiViz.prevPos = here
			vestiViz.prevX = X
			vestiViz.prevY = Y
			vestiViz.start = false
		else
			local dt = now - vestiViz.prevTime;
		
			if dt > 0 then
				
				vel = vestiViz.vecDiff(vestiViz.prevPos,here,dt)
				
				local worldAcc = vestiViz.vecDiff(vestiViz.prevVel,vel,dt)
				worldAcc.y = worldAcc.y + 9.81
				
				local decayFactor = math.pow(0.5,dt/vestiViz.halflife)
				
				vestiViz.addToTail(vestiViz.acc,
									{x = vestiViz.vecDot(worldAcc,X),
									 y = vestiViz.vecDot(worldAcc,Y) - 9.81,
									 z = vestiViz.vecDot(worldAcc,pos3.z)},
									decayFactor
								)
								 
				local viewAcc = vestiViz.compwiseMult(vestiViz.acc,vestiViz.accFactors)
				local somatograv = vestiViz.normalize(vestiViz.acc)

				if viewAcc.y < 0 then
					viewAcc.y = viewAcc.y * vestiViz.accFactors.mY
				end

				local dX = vestiViz.vecDiff(vestiViz.prevX,X,dt)

				local dY = vestiViz.vecDiff(vestiViz.prevY,Y,dt)
				
				vestiViz.addToTail(vestiViz.rot,
									{x = vestiViz.vecDot(pos3.z,dY),
									 y = vestiViz.vecDot(pos3.z,dX),
									 z = vestiViz.vecDot(Y,dX)},
									decayFactor
								)
								
				local viewRot = vestiViz.compwiseMult(vestiViz.rot,vestiViz.rotFactors)
							 
				local bottom = vestiViz.normalizeBar({
					off = -viewRot.x + viewRot.y + somatograv.z * vestiViz.somatogravFactor,
					w = viewAcc.y + viewAcc.x
				})
						  
				local top = vestiViz.normalizeBar({
					off = viewRot.x + viewRot.y - somatograv.z * vestiViz.somatogravFactor,
					w = -viewAcc.y + viewAcc.x
				})
						  
				local left = vestiViz.normalizeBar({
					off = - viewRot.x + viewRot.z - (somatograv.x + somatograv.z) * vestiViz.somatogravFactor,
					w = viewAcc.z + viewAcc.x
				})
				
				local right = vestiViz.normalizeBar({
					off = viewRot.x + viewRot.z + (somatograv.z - somatograv.x) * vestiViz.somatogravFactor,
					w = - viewAcc.z + viewAcc.x
				})
				
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
					
				vestiViz.prevTime = now
				vestiViz.prevVel = vel
				vestiViz.prevPos = here
				vestiViz.prevX = X
				vestiViz.prevY = Y
				
			end
		end

	end
end

DCS.setUserCallbacks(vestiViz)