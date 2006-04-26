<?php
	require_once('config.inc.php');
	global $SETTINGS;
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
	<head>
		<title>Wolfpack Reference</title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
		<link href="./style.css" rel="stylesheet" type="text/css" />
	</head>
	<body>
<?php require_once('header.inc.php'); ?>
	<div align="center">
		<br />
		<span class="maintitle">Wolfpack Reference</span><br />
		<br />
		<table width="740" border="0" cellspacing="3" cellpadding="2">
			<tr align="left" valign="top">
				<td width="33%" class="text">
					<div align="left">
						<span class="sectiontitle">INGAME COMMANDS</span><br />
						These commands can be used by players, gms and admins while in the game. This section gives you an overview over all available commands and how to use them.<br />
						<a href="command.php">Read more... (<?
						$result = mysql_query("SELECT COUNT(*) FROM documentation_commands;");
						echo array_pop(mysql_fetch_array($result));
						mysql_free_result($result);
						?> commands)</a><br />
						<br />
					</div>
				</td>
				<td width="33%" class="text">
					<span class="sectiontitle">SCRIPTING OBJECTS</span><br />
					This section of the documentation gives you an overview over available wolfpack specific objects, their properties and methods and in some cases how to create them.<br />
					<a href="object.php">Read more... (<?
						$result = mysql_query("SELECT COUNT(*) FROM documentation_objects;");
					echo array_pop(mysql_fetch_array($result));
					mysql_free_result($result);
					?> objects)</a>
				</td>
				<td width="33%" class="text">
					<span class="sectiontitle">SCRIPTING LIBRARY</span><br />
					Wolfpack comes with a sophisticated library of scripts. This section of the documentation provides an overview over the library and its functions.<br />
					<a href="#">Read more... </a>
				</td>
			</tr>
		</table>
		<hr width="740" size="1" noshade="noshade" />
		<table width="740" border="0" cellpadding="2" cellspacing="3">
			<tr align="left" valign="top">
				<td width="33%" class="text">
					<span class="sectiontitle">SCRIPTING EVENTS</span><br />
					This section describes the events your scripts will be notified about.<br />
					<a href="event.php">Read more... (<?
					$result = mysql_query("SELECT COUNT(*) FROM documentation_events;");
					echo array_pop(mysql_fetch_array($result));
					mysql_free_result($result);
					?> events)</a><br />
				</td>
				<td width="33%" class="text">
					<span class="sectiontitle">DEFINITION TAGS</span><br />
					This section provides an overview over the tags used in the Wolfpack XML definitions.
					<a href="#">Read more...</a>
				</td>
				<td width="33%" class="text">
					<span class="sectiontitle">MISCELLANEOUS</span><br />
					<a href="introduction.html">Introduction to Wolfpack</a><br />
					<a href="FAQ.html">FAQ</a><br />
					<a href="http://doc.wpdev.org/ChangeLog.wolfpack">Changelog</a><img src="external_link.gif" alt="External Link" width="11" height="12" border="0" align="bottom" /><br />
					<a href="http://doc.wpdev.org/formats/">File Formats</a> <img src="external_link.gif" alt="External Link" width="11" height="12" border="0" align="bottom" /><br />
					<a href="http://doc.wpdev.org/wolfpack-{SHORTVERSION}-doc.zip">Download Documentation</a> <img src="external_link.gif" alt="External Link" width="11" height="12" border="0" align="bottom" />
				</td>
			</tr>
		</table>
		<br />
<!-- SiteSearch Google -->
<FORM method=GET action="http://www.google.com/search">
<input type=hidden name=ie value=UTF-8>
<input type=hidden name=oe value=UTF-8>
<TABLE bgcolor="#FFFFFF"><tr><td>
<A HREF="http://www.google.com/">
<IMG SRC="http://www.google.com/logos/Logo_40wht.gif" 
border="0" ALT="Google"></A>
</td>
<td>
<INPUT TYPE=text name=q size=31 maxlength=255 value="">
<INPUT type=submit name=btnG VALUE="Google Search">
<font size=-1>
<input type=hidden name=domains value="doc.wpdev.org"><br><input type=hidden name=sitesearch value="doc.wpdev.org">Search Wolfpack Reference<br>
</font>
</td></tr></TABLE>
</FORM>
<!-- SiteSearch Google -->
<?php require_once('footer.inc.php'); ?>
		</div>
	</body>
</html>
