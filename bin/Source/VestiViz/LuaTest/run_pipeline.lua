
package.cpath = package.cpath..";"..BinDir.."\\?.dll;"


-- LOAD DLL
--VestiViz.log(_VERSION)
vestiviz = require('vestiviz')

	Handle = vestiviz.Start()

	vestiviz.Stop(Handle)