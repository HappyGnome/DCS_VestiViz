
package.cpath = package.cpath..";"..BinDir.."\\?.dll;"


-- LOAD DLL
--VestiViz.log(_VERSION)
vestiviz = require('vestiviz')

	Handle = vestiviz.Start()

	for i=1,1000,1 do
		vestiviz.AddDatum(Handle,i,
		{p = {x = 1, y= 2, z = 3},
		x = {x = 1.1, y= 2.1, z = 3.1},
		y = {x = 1.2, y= 2.2, z = 3.2},
		z = {x = 1.3, y= 2.3, z = 3.3}});
		foo = vestiviz.GetDatum(Handle);
		print(i..":"..foo.t);
	end


	vestiviz.Stop(Handle)