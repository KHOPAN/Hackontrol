package com.khopan.hackontrol.command;

import com.khopan.hackontrol.permission.Permission;

public interface BulletCommand extends Command {
	@Override
	default Permission getPermissionLevel() {
		return Permission.BULLET;
	}
}
