package com.khopan.hackontrol.security;

import java.util.List;

import net.dv8tion.jda.api.Permission;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.Member;
import net.dv8tion.jda.api.entities.Role;
import net.dv8tion.jda.api.managers.channel.attribute.IPermissionContainerManager;

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

	public static void configureViewPermission(IPermissionContainerManager<?,?> manager) {
		PermissionType[] allowedTypes = new PermissionType[] {PermissionType.ROOT, PermissionType.VIEW};
		Guild guild = manager.getGuild();
		long everyoneRole = guild.getPublicRole().getIdLong();
		manager.putRolePermissionOverride(everyoneRole, null, List.of(Permission.VIEW_CHANNEL));

		for(PermissionType type : allowedTypes) {
			manager.putRolePermissionOverride(type.getRole(guild).getIdLong(), List.of(Permission.VIEW_CHANNEL), null);
		}

		manager.queue();
	}
}
