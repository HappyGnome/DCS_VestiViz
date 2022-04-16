declare_plugin("VestiViz", {
	installed = true,
	dirName = current_mod_path,
	developerName = _("Benom8"),
	--developerLink = "",
	displayName = _("VestiViz"),
	version = "1.0",
	state = "installed",
	info = _("VestiViz\n\nAdds an overlay to give a representation of the information you would get from your vestibular system and the proverbial 'seat of the pants' as you fly."),
	binaries = {"vestiviz.dll"},
    load_immediate = true,
	Skins = {
		{ name = "VestiViz", dir = "Theme" },
	}--,
	--Options = {},
})

plugin_done()