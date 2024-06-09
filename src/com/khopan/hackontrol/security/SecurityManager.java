package com.khopan.hackontrol.security;

import net.dv8tion.jda.api.entities.Member;
import net.dv8tion.jda.api.entities.Role;

public class SecurityManager {
	private SecurityManager() {}

	public static boolean checkPermission(IPermissive object, Member member) {
		if(member == null) {
			return false;
		}

		if(member.isOwner() || PermissionType.ROOT.hasPermission(member)) {
			return true;
		}

		if(object == null) {
			return false;
		}

		PermissionType[] requiredPermission = object.requiredPermission();

		if(requiredPermission == null) {
			return true;
		}

		for(Role role : member.getRoles()) {
			for(PermissionType permission : requiredPermission) {
				if(permission.checkRole(role)) {
					return true;
				}
			}
		}

		return false;
	}
}
