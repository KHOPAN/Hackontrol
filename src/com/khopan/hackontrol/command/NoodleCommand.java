package com.khopan.hackontrol.command;

import com.khopan.hackontrol.permission.Permission;

public interface NoodleCommand extends Command {
	@Override
	default Permission getPermissionLevel() {
		return Permission.NOODLE;
	}
}
