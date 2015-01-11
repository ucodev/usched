<?php
	if (usc_request("run 'ls -lah /' in 10 seconds then every 5 seconds") !== TRUE)
		die("usc_request(): Failed.");

	$entry_list = usc_result_get_run();

	foreach ($entry_list as $entry)
		echo("Installed entry: " . $entry);

	usc_result_free_run();

