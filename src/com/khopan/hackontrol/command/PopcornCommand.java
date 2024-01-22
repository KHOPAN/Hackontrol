package com.khopan.hackontrol.command;

import com.khopan.hackontrol.permission.Permission;

public interface PopcornCommand extends Command {
	@Override
	default Permission getPermissionLevel() {
		return Permission.POPCORN;
	}
}
