package com.khopan.hackontrol.permission;

import java.util.List;

import com.khopan.hackontrol.Hackontrol;

import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.Member;
import net.dv8tion.jda.api.entities.Role;
import net.dv8tion.jda.api.entities.User;
import net.dv8tion.jda.api.exceptions.ErrorResponseException;

public class PermissionManager {
	private PermissionManager() {}

	public static Permission getPermission(User user) {
		long userIdentifier = user.getIdLong();
		Hackontrol hackontrol = Hackontrol.getInstance();
		Guild guild = hackontrol.getGuild();
		Member member;

		try {
			member = guild.retrieveMemberById(userIdentifier).complete();
		} catch(ErrorResponseException Exception) {
			return Permission.POPCORN;
		}

		List<Role> roleList = member.getRoles();
		Permission[] permissions = Permission.values();
		Permission highestPermission = Permission.POPCORN;
		int lowestPermissionLevel = Integer.MAX_VALUE;

		for(int x = 0; x < roleList.size(); x++) {
			Role role = roleList.get(x);
			long roleIdentifier = role.getIdLong();

			for(int y = 0; y < permissions.length; y++) {
				if(permissions[y].getIdentifier() == roleIdentifier) {
					int ordinal = permissions[y].ordinal();

					if(ordinal < lowestPermissionLevel) {
						lowestPermissionLevel = ordinal;
						highestPermission = permissions[y];
					}

					break;
				}
			}
		}

		return highestPermission;
	}
}
