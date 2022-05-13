
package.cpath = package.cpath..";"..BinDir.."\\?.dll;"

-- LOAD DLL
--VestiViz.log(_VERSION)
vestiviz = require('vestiviz')

foo = function()
	local pipeline = vestiviz.newPipeline();

	local frameinput1, frameinput2;
	local leaf1,input1 = pipeline.accelByRegressionFilterPoint();
	leaf1 = pipeline.staticAddFilterPoint({x = 0, y = 9.81, z = 0},leaf1);
	leaf1, frameinput1 = pipeline.dynMatMultFilterPoint(leaf1,nil);
	print(leaf1..":"..frameinput1);
	leaf1 = pipeline.staticAddFilterPoint({x = 0, y = -9.81, z = 0},leaf1);
	leaf1 = pipeline.quickCompressFilterPoint({x = 1, y = 1, z = 1},leaf1);
	leaf1 = pipeline.expDecayFilterPoint(100.0,leaf1);
	leaf1 = pipeline.matMultFilterPointToWOff({
					0.5, -0.5, 0.0,--T width
					0.5, 0.0, -0.5,--R width
					0.5, 0.5, 0.0,--B width
					0.5, 0.0, 0.5,--L width
					0.0, 0.0, 1.0, --T somatograv
					1.0, 0.0, 1.0,--R
					0.0, 0.0, -1.0,--B
					1.0, 0.0, -1.0},
					leaf1);
	local leaf2, input2 = pipeline.simpleDiffFilterXY();
	leaf2,frameinput2= pipeline.dynMatMultPickFilterXYtoPoint({
					{2,1}, --x-axis rot
					{2,0}, --negative y-axis rot
					{1,0}} --z-axis rot
					,leaf2);
	leaf2 = pipeline.quickCompressFilterPoint({x = 1, y = 1, z = 1},leaf2);
	leaf2 = pipeline.expDecayFilterPoint(100.0,leaf2);
	leaf2 = pipeline.matMultFilterPointToWOff(
					{0.0, 0.0, 0.0,--T width
					0.0, 0.0, 0.0,--R
					0.0, 0.0, 0.0,--B
					0.0, 0.0, 0.0,--L 
					-1.0, 1.0, 0.0, --T displacement
					-1.0, 0.0, 1.0,--R
					1.0, 1.0, 0.0,--B
					1.0, 0.0, 1.0},
					leaf2);
	local leaf3 = pipeline.linCombFilterWOff(0.5,0.5,leaf1,leaf2);
	leaf3 = pipeline.quickCompressFilterWOff({w = {top = 1,right = 1,bottom = 1,left = 1}, off = {top = 1,right = 1,bottom = 1,left = 1}},leaf3);
	leaf3 = pipeline.convolveOutputFilterWOff({0.25,0.5,0.25},leaf3,3);
	local output = pipeline.makeWOffOutput(leaf3);

	--local output = pipeline.makePointOutput(leaf1);

	local error = pipeline.popError();
	while error ~= nil do
		print(error);
		error = pipeline.popError();
	end
	print("Starting "..input1.." "..input2.." "..frameinput1.." "..frameinput2);
	pipeline.start();
	for i=1,1,1 do
		pipeline.addDatum(input1,i, {p = {x = 1, y= 2, z = 3}});
		pipeline.addDatum(input2,i, {x = {x = 1.1, y= 2.1, z = 3.1},
																 y = {x = 1.2, y= 2.2, z = 3.2}});
		pipeline.addDatum(frameinput1,i, {x = {x = 1.1, y= 2.1, z = 3.1},
																			y = {x = 1.2, y= 2.2, z = 3.2},
																			z = {x = 1.3, y= 2.3, z = 3.3}});
		pipeline.addDatum(frameinput2,i, {x = {x = 1.1, y= 2.1, z = 3.1},
																			y = {x = 1.2, y= 2.2, z = 3.2},
																			z = {x = 1.3, y= 2.3, z = 3.3}});
		--print(i.." add");
	end
	--print("slow");
	--if (pcall( function()
		for i=1,59,1 do
			local foo = pipeline.getDatum(output);
			print(i..":"..foo.t..", "..foo.w.top);
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
print(collectgarbage("collect"));