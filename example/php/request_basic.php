<?php
	include("usched.php");

	$usc = new Usched();

	if ($usc->Request("run 'ls -lah /' in 10 seconds then every 5 seconds") !== TRUE)
		die("usc->Request(): Failed.");

	$entry_list = $usc->ResultRun();

	foreach ($entry_list as $entry)
		echo("Installed entry: " . $entry);

