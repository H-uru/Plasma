-- Export function, takes an input dir (where max scene files are),
-- and an output dir, where the exported files should go
--

function ExportScenes   =
(
	scnInDir = GetSavePath caption:"Choose Max Source Directory"
	if scnInDir == undefined then return "Cancel"
	scnInDir = scnInDir + "\\"
	
	scnOutFile = GetSaveFileName caption:"Choose Dest File" types:"PRD(*.prd)|*.prd|"
	if scnOutFile == undefined then return "Cancel"		 
	scnOutDir = getFileNamePath(scnOutFile) + "dat\\"

	makeDir scnOutDir
	files = getFiles (scnInDir + "*.max")
	start = timeStamp()
	for f in files do
	(
		format "Exporting file % to %\n" f scnOutFile 
		loadMAXFile f
		exportFile scnOutFile #noPrompt
	)
	end = TimeStamp()
	format "Exporting took % minutes\n" (( end - start) / 60000.0)
)


--
-- main
--
utility main_panel "Build Data"
(
	button convert_scenes "Plasma Exporter"
	on convert_scenes pressed do
	(
		ExportScenes ()
	)
)