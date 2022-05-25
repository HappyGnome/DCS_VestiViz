
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
		halflife = 0.2,
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

	local filter1, innerleaf1 = pipeline.simpleDiffFilterPoint();
	filter1, innerleaf1 = pipeline.staticAddFilterPoint({x = 0, y = 9.81, z = 0}, filter1, innerleaf1);
	filter1, innerleaf1 = pipeline.dynMatMultFilterPoint(filter1, innerleaf1,nil);

	print(innerleaf1..":"..filter1);
	filter1, innerleaf1 = pipeline.staticAddFilterPoint({x = 0, y = -9.81, z = 0},filter1, innerleaf1);
	filter1, innerleaf1 = pipeline.signScaleFilterPoint({x = 0, y = 2, z = 3},filter1, innerleaf1);
	
	pipeline.addInputToFilter(filter1, innerleaf1);

	filter1, innerleaf1 = pipeline.quickCompressFilterPoint(config.acclims,filter1, innerleaf1);
	filter1, innerleaf1 = pipeline.expDecayFilterPoint(config.halflife,filter1, innerleaf1);
	filter1, innerleaf1 = pipeline.matMultFilterPointToWOff({
					-a, -a, 0.0,--T width
					-a, 0.0, -a,--R width
					-a, a, 0.0,--B width
					-a, 0.0, a,--L width
					0.0, 0.0, s, --T somatograv
					s, 0.0, s,--R
					0.0, 0.0, -s,--B
					s, 0.0, -s},
					filter1, innerleaf1);

	local outputHandle1, inputHandle1, inputHandle2 = pipeline.connectFilter(filter1)
	print(inputHandle2)
	local inputPoint = pipeline.makePointInput(inputHandle1);
  local inputFrame1 = pipeline.makeFrameInput(inputHandle2);

	local filter2, innerleaf2 = pipeline.simpleDiffFilterXY();
	filter2, innerleaf2 = pipeline.dynMatMultPickFilterXYtoPoint({
					{2,1}, --x-axis rot
					{2,0}, --negative y-axis rot
					{1,0}} --z-axis rot
					,filter2, innerleaf2);
	filter2, innerleaf2 = pipeline.quickCompressFilterPoint(config.rotlims,filter2, innerleaf2);
	filter2, innerleaf2 = pipeline.expDecayFilterPoint(config.halflife,filter2, innerleaf2);
	filter2, innerleaf2 = pipeline.matMultFilterPointToWOff(
					{0.0, 0.0, 0.0,--T width
					0.0, 0.0, 0.0,--R
					0.0, 0.0, 0.0,--B
					0.0, 0.0, 0.0,--L 
					-r, -r, 0.0, --T displacement
					-r, 0.0, r,--R
					r, -r, 0.0,--B
					r, 0.0, r},
					filter2, innerleaf2);

	local outputHandle2, inputHandle3, inputHandle4 = pipeline.connectFilter(filter2)
	print(inputHandle4)
	local inputXY = pipeline.makeXYInput(inputHandle3);
  local inputFrame2 = pipeline.makeFrameInput(inputHandle4);
	--
	local filter3, innerleaf3 = pipeline.linCombFilterWOff(2,2);
	filter3, innerleaf3 = pipeline.quickCompressFilterWOff({w = {top = 1,right = 1,bottom = 1,left = 1}, off = {top = 1,right = 1,bottom = 1,left = 1}},filter3, innerleaf3);
	filter3, innerleaf3 = pipeline.convolveOutputFilterWOff({0.25,0.5,0.25},filter3, innerleaf3);

	local outputHandle3 = pipeline.connectFilter(filter3, outputHandle1, outputHandle2)
	print(outputHandle3)

	local output = pipeline.makeWOffOutput(outputHandle3);

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
	for i=1,10,1 do
		pipeline.addDatum(inputPoint,i, {p = {x = 0, y= 0, z = i*i}});
		pipeline.addDatum(inputXY,i, {x = {x = 1.1, y= 2.1, z = 3.1},
																 y = {x = 1.2, y= 2.2, z = 3.2}});
		pipeline.addDatum(inputFrame1,i, {x = {x = 1, y= 0, z =0},
																			y = {x = 0, y= 1, z = 0},
																			z = {x = 0, y= 0, z = 1}});
		pipeline.addDatum(inputFrame2,i, {x = {x = 1, y= 0, z =0},
																			y = {x = 0, y= 1, z = 0},
																			z = {x = 0, y= 0, z = 1}});
		--print(i.." add");
	end
	--print("slow");
	--if (pcall( function()
		for i=1,59,1 do
			local foo = pipeline.getDatum(output);
			log(foo)
			--print(i);
		end
	--end)~= true ) then print ("error") end

	local error = pipeline.popError();
	while error ~= nil do
		print(error);
		error = pipeline.popError();
	end


	pipeline.stop();
end
foo();