package com.khopan.hackontrol.command;

import com.khopan.hackontrol.permission.Permission;

public interface SparkCommand extends Command {
	@Override
	default Permission getPermissionLevel() {
		return Permission.SPARK;
	}
}
