<?php
	include("usched.php");

	$usc = new Usched();

	if ($usc->Request("show all") !== TRUE)
		die("usc->Request(): Failed.");

	$entry_list = $usc->ResultShow();

	foreach ($entry_list as $id => $entry) {
		echo("Entry ID: " . $id . "<br />");
		echo("&nbsp;&nbsp;Username: " . $entry_list[$id]["username"] . "<br />");
		echo("&nbsp;&nbsp;UID: " . $entry_list[$id]["uid"] . "<br />");
		echo("&nbsp;&nbsp;GID: " . $entry_list[$id]["gid"] . "<br />");
		echo("&nbsp;&nbsp;Trigger: " . $entry_list[$id]["trigger"] . "<br />");
		echo("&nbsp;&nbsp;Step: " . $entry_list[$id]["step"] . "<br />");
		echo("&nbsp;&nbsp;Expire: " . $entry_list[$id]["expire"] . "<br />");
		echo("&nbsp;&nbsp;Command: " . $entry_list[$id]["cmd"] . "<br />");
		echo("<br />");
	}

