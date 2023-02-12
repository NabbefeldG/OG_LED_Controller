Set oShell = CreateObject ("Wscript.Shell")
Dim strArgs
strArgs = "cmd /c OG_LED_Controller.bat"
oShell.Run strArgs, 0, false