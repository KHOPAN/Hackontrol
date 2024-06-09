package com.khopan.hackontrol.utils;

import com.khopan.hackontrol.security.IPermissive;
import com.khopan.hackontrol.security.SecurityManager;
import com.khopan.hackontrol.utils.sendable.ISendable;

import net.dv8tion.jda.api.entities.Member;

public class HackontrolPermission {
	private HackontrolPermission() {}

	public static boolean checkPermission(IPermissive object, Member member, ISendable sender) {
		if(SecurityManager.checkPermission(object, member)) {
			return true;
		}

		HackontrolMessage.boldDeletable(sender, "Access denied: Not enough permission");
		return false;
	}
}
