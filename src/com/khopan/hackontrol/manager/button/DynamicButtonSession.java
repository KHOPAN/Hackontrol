package com.khopan.hackontrol.manager.button;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;

public class DynamicButtonSession {
	static final List<DynamicButtonSession> SESSION_LIST = new ArrayList<>();

	long sessionIdentifier;
	Consumer<ButtonContext> action;
	Object[] paramters;
	long messageIdentifier;

	DynamicButtonSession() {}

	static long randomSessionIdentifier() {
		long sessionIdentifier = 0L;

		do {
			sessionIdentifier = Math.round(Math.random() * Long.MAX_VALUE);
		} while(DynamicButtonSession.hasCollision(sessionIdentifier));

		return sessionIdentifier;
	}

	static DynamicButtonSession decodeSession(String identifier) {
		if(!identifier.startsWith("bm-sid")) {
			return null;
		}

		long sessionIdentifier;

		try {
			sessionIdentifier = Long.parseLong(identifier.substring(6).trim());
		} catch(Throwable Errors) {
			Hackontrol.LOGGER.warn("Invalid button identifier: {}", identifier);
			return null;
		}

		DynamicButtonSession session = DynamicButtonSession.findSession(sessionIdentifier);

		if(session == null) {
			Hackontrol.LOGGER.warn("No session found for identifier: {}", identifier);
			return null;
		}

		return session;
	}

	private static DynamicButtonSession findSession(long sessionIdentifier) {
		for(int i = 0; i < DynamicButtonSession.SESSION_LIST.size(); i++) {
			DynamicButtonSession session = DynamicButtonSession.SESSION_LIST.get(i);

			if(session.sessionIdentifier == sessionIdentifier) {
				return session;
			}
		}

		return null;
	}

	private static boolean hasCollision(long sessionIdentifier) {
		for(int i = 0; i < DynamicButtonSession.SESSION_LIST.size(); i++) {
			DynamicButtonSession session = DynamicButtonSession.SESSION_LIST.get(i);

			if(session.sessionIdentifier == sessionIdentifier) {
				return true;
			}
		}

		return false;
	}
}
