
package.cpath = package.cpath..";"..BinDir.."\\?.dll;"

-- LOAD DLL
--VestiViz.log(_VERSION)
vestiviz = require('vestiviz')

foo = function()
	local pipeline = vestiviz.newPipeline();

	pipeline.start();
	for i=1,1000,1 do
		pipeline.addDatum(i,
		{p = {x = 1, y= 2, z = 3},
		x = {x = 1.1, y= 2.1, z = 3.1},
		y = {x = 1.2, y= 2.2, z = 3.2},
		z = {x = 1.3, y= 2.3, z = 3.3}});
	end
	for i=1,1000,1 do
		foo = pipeline.getDatum();
		print(i..":"..foo.t);
	end
	pipeline.stop();
end
foo();
print(collectgarbage("collect"));