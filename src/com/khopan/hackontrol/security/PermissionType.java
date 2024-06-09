package com.khopan.hackontrol.security;

import java.util.List;

import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.Member;
import net.dv8tion.jda.api.entities.Role;

public enum PermissionType {
	ROOT("Root"),
	DELETABLE("Deletable"),
	VIEW("View");

	private final String name;

	PermissionType(String name) {
		this.name = name;
	}

	public String getName() {
		return this.name;
	}

	public boolean checkRole(Role role) {
		return role.getName().equalsIgnoreCase(this.name);
	}

	public boolean hasPermission(Member member) {
		for(Role role : member.getRoles()) {
			if(this.checkRole(role)) {
				return true;
			}
		}

		return false;
	}

	public Role getRole(Guild guild) {
		List<Role> list = guild.getRolesByName(this.name, true);

		if(!list.isEmpty()) {
			return list.get(0);
		}

		return guild.createRole().setName(this.name).complete();
	}
}
