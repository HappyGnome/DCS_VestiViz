  --Hook to load VestiVis GUI
 local status, result = pcall(function() local dcsSr=require('lfs');dofile(dcsSr.writedir()..[[Mods\Services\VestiViz\Scripts\DCS-VestiViz-Overlay.lua]]); end,nil) 
 
 if not status then
 	net.log(result)
 end