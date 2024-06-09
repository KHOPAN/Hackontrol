package com.khopan.hackontrol.security;

public class Permissive implements IPermissive {
	public static final IPermissive DELETE_BUTTON = new Permissive(PermissionType.DELETABLE);

	private final PermissionType[] types;

	private Permissive(PermissionType... types) {
		this.types = types;
	}

	@Override
	public PermissionType[] requiredPermission() {
		return this.types;
	}
}
