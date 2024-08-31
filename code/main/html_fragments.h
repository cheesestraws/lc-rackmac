#pragma once

#define HTML_TOP \
"<!doctype html>" \
"<html>" \
"<head>" \
"<title>" \
"LCBMC" \
"</title>" \
"<style>" \
"body {" \
"	font-family: sans-serif;" \
"	padding: 32px;" \
"	background-color: #ffffff;" \
"	background-image:  repeating-linear-gradient(45deg, #000000 25%, transparent 25%, transparent 75%, #000000 75%, #000000), repeating-linear-gradient(45deg, #000000 25%, #ffffff 25%, #ffffff 75%, #000000 75%, #000000);" \
"	background-position: 0 0, 1px 1px;" \
"	background-size: 2px 2px;" \
"}" \
"form {" \
"	padding: none;" \
"	margin: none;" \
"	padding-left: 4em;" \
"	padding-bottom: 0.5em;" \
"}" \
".d1 {" \
"	background: white;" \
"	border: 1px solid black;" \
"	padding: 2px;" \
"	width: 512px;" \
"}" \
".d1 hr {" \
"	border: none;" \
"	border-top:	 3px double black;" \
"}" \
".d2 {" \
"	border: 2px solid black;" \
"	padding: 0.5em;" \
"}" \
".btns {" \
"	text-align: right;" \
"}" \
"</style>" \
"</head>" \
"</body>" \
"<div class='d1'>" \
"<div class='d2'>"

#define HTML_BOTTOM \
"</div>" \
"</div>" \
"</body>" \
"</html>"

#define OK_BTN \
"<div style='text-align:right'>" \
"<a href='/'><button type='button'>OK</button></a>" \
"</div>"


