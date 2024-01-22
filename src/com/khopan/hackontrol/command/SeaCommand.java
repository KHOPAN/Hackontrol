package com.khopan.hackontrol.command;

import com.khopan.hackontrol.permission.Permission;

public interface SeaCommand extends Command {
	@Override
	default Permission getPermissionLevel() {
		return Permission.SEA;
	}
}
