
package.cpath = package.cpath..";"..BinDir.."\\?.dll;"

-- LOAD DLL
--VestiViz.log(_VERSION)
vestiviz = require('vestiviz')


config = {
		enabled = true,
		barWidth = 10,
		accFactor = 1,
		rotFactor = 1,
		somatogravFactor  = 1,
		maxw = 0.3,
		minw = 0.0,
		offlim = 0.4,
		halflife = 0.1,
		acclims = {x = 5,y = 20,z = 5, mY = 2},
		rotlims = {x = 0.5,y = 0.5,z = 0.5},
		logHotkey = "Ctrl+Alt+v",
		showHotkey = "Ctrl+Shift+v",
		incHotkey = "Ctrl+Shift+w",
		decHotkey = "Ctrl+Alt+w",
		colours = {"0xffdd00ee","0xffffff88","0x44ff4444","0x22ff2222","0x11ff1111","0x00ff0008"}
	}

tbl2str = function(tbl)
	local msg = '{'
			for k,v in pairs(tbl) do
				local t = type(v)
				if t == 'string' or t == 'number' then
					msg = msg..k..':'..v..', '
				elseif type(tbl) == 'table' then
					msg = msg..k..':'..tbl2str(v)..', '
				else
					msg = msg..k..':'..t..', '
				end
			end
			return msg..'}'
end

log = function(str)
    if not str then 
        return
    end
		local msg
		if type(str) == 'table' then
			msg = tbl2str(str)
		else
			msg = str
		end
		print(msg)
end

foo = function()
	local pipeline = vestiviz.newPipeline();

	local s = config.somatogravFactor
	local a = config.accFactor
	local r = config.rotFactor

		local filter1, innerleaf1 = pipeline.simpleDiffFilterPoint(); --Input 1
	filter1, innerleaf1 = pipeline.staticAddFilterPoint({x = 0, y = 9.81, z = 0}, filter1, innerleaf1);

	pipeline.addOutputToFilter(filter1, innerleaf1);

	local filter1a, innerleaf1a = pipeline.dynMatMultFilterPoint(filter1, innerleaf1,nil); -- somatogravic --Input 1a
	local filter1b, innerleaf1b = pipeline.dynMatMultFilterPoint(filter1, innerleaf1,nil); -- chair sense --Input 1b

	filter1a, innerleaf1a = pipeline.staticAddFilterPoint({x = 0, y = -9.81, z = 0},filter1a, innerleaf1a);
	filter1a, innerleaf1a = pipeline.quickCompressFilterPoint(config.acclims,filter1a, innerleaf1a);

	filter1a, innerleaf1a = pipeline.matMultFilterPointToW({
					0.0, 0.0, s, --T somatograv
					s, 0.0, s,--R
					0.0, 0.0, -s,--B
					s, 0.0, -s},
					filter1a, innerleaf1a);

	filter1b, innerleaf1b = pipeline.staticAddFilterPoint({x = 0, y = -9.81, z = 0},filter1b, innerleaf1b);
	filter1b, innerleaf1b = pipeline.signScaleFilterPoint({x = 1, y = config.acclims.mY, z = 1},filter1b, innerleaf1b);
	filter1b, innerleaf1b = pipeline.quickCompressFilterPoint(config.acclims,filter1b, innerleaf1b);

	
	filter1b, innerleaf1b = pipeline.matMultFilterPointToW({
					-a, -a, 0.0,--T width
					-a, 0.0, -a,--R width
					-a, a, 0.0,--B width
					-a, 0.0, a},--L width
					filter1b, innerleaf1b);

	local outputHandle1, inputHandle1, inputHandle1a, inputHandle1b = pipeline.connectFilter(filter1)

	local inputVel = pipeline.makePointInput(inputHandle1);
  	local inputFrame1a = pipeline.makeFrameInput(inputHandle1a); -- head relative frame
	local inputFrame1b = pipeline.makeFrameInput(inputHandle1b); -- ship frame

	local filter2, innerleaf2 = pipeline.simpleDiffFilterXY(); -- Input 2
	filter2, innerleaf2 = pipeline.dynMatMultPickFilterXYtoPoint({
					{2,1}, --x-axis rot
					{2,0}, --negative y-axis rot
					{1,0}} --z-axis rot
					,filter2, innerleaf2); -- Input 2a
	filter2, innerleaf2 = pipeline.quickCompressFilterPoint(config.rotlims,filter2, innerleaf2);
	filter2, innerleaf2 = pipeline.matMultFilterPointToW(
					{-r, -r, 0.0, --T displacement
					-r, 0.0, r,--R
					r, -r, 0.0,--B
					r, 0.0, r},
					filter2, innerleaf2);

	local outputHandle2, inputHandle2, inputHandle2a = pipeline.connectFilter(filter2)

	local inputXY = pipeline.makeXYInput(inputHandle2);
  	local inputFrame2a = pipeline.makeFrameInput(inputHandle2a); -- head relative frame
	--
	local filter3, innerleaf3 = pipeline.linCombFilterW(2,2); -- Input 3a & 3b -- Combine somatograv and vestibular

	filter3, innerleaf3 = pipeline.concatWToWOff(filter3, nil,innerleaf3); -- Input 3c 
	filter3, innerleaf3 = pipeline.quickCompressFilterWOff({w = {top = 1,right = 1,bottom = 1,left = 1}, off = {top = 1,right = 1,bottom = 1,left = 1}},filter3, innerleaf3);
	filter3, innerleaf3 = pipeline.expDecayFilterWOff(config.halflife,filter3, innerleaf3);
	filter3, innerleaf3 = pipeline.convolveOutputFilterWOff({0.25,0.5,0.25},filter3, innerleaf3);

	local outputHandle3 = pipeline.connectFilter(filter3, outputHandle1, outputHandle2, outputHandle1)

	local output = pipeline.makeWOffOutput(outputHandle3,true);

	if pipeline.validate() then
		print("Validation pass" );
	else
		print("Validation fail" );
	end

	

	local error = pipeline.popError();
	while error ~= nil do
		print(error);
		error = pipeline.popError();
	end
	pipeline.start();
 local sec = tonumber(os.clock() + 1); 
    while (os.clock() < sec) do 
    end 
	for i=1,10,1 do
		pipeline.addDatum(inputVel,i, {p = {x = 0, y= -50*i*i, z = -i*i}});
		pipeline.addDatum(inputXY,i, {x = {x = 1.1, y= 2.1, z = 3.1},
																 y = {x = 1.2, y= 2.2, z = 3.2}});
		pipeline.addDatum(inputFrame1a,i, {x = {x = 1, y= 0, z =0},
																			y = {x = 0, y= 1, z = 0},
																			z = {x = 0, y= 0, z = 1}});
		pipeline.addDatum(inputFrame1b,i, {x = {x = 1, y= 0, z =0},
																			y = {x = 0, y= 1, z = 0},
																			z = {x = 0, y= 0, z = 1}});
		pipeline.addDatum(inputFrame2a,i, {x = {x = 1, y= 0, z =0},
																			y = {x = 0, y= 1, z = 0},
																			z = {x = 0, y= 0, z = 1}});
		print("slow");
			local foo = pipeline.getDatum(output);
			log(foo)
	end
	--print("slow");
	--if (pcall( function()
		--[[for i=1,59,1 do
			local foo = pipeline.getDatum(output);
			log(foo)
			--print(i);
		end]]
	--end)~= true ) then print ("error") end

	local error = pipeline.popError();
	while error ~= nil do
		print(error);
		error = pipeline.popError();
	end


	pipeline.stop();
end
foo();