package com.khopan.hackontrol.service.interaction;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;

class InteractionSession {
	static final List<InteractionSession> SESSION_LIST = new ArrayList<>();

	private static final String PREFIX = "ims";

	long sessionIdentifier;
	long messageIdentifier;
	Object[] parameters;
	Consumer<?> action;
	InteractionType type;

	InteractionSession() {}

	static long newSession() {
		long sessionIdentifier = 0L;

		do {
			sessionIdentifier = Math.round(Math.random() * Long.MAX_VALUE);
		} while(InteractionSession.hasCollision(sessionIdentifier));

		return sessionIdentifier;
	}

	static String prefix(long sessionIdentifier) {
		return InteractionSession.PREFIX + sessionIdentifier;
	}

	static InteractionSession decodeSession(String identifier) {
		if(identifier.startsWith("imsd") || !identifier.startsWith(InteractionSession.PREFIX)) {
			return null;
		}

		long sessionIdentifier;

		try {
			sessionIdentifier = Long.parseLong(identifier.substring(InteractionSession.PREFIX.length()).trim());
		} catch(Throwable Errors) {
			Hackontrol.LOGGER.warn("Invalid session identifier: {}", identifier);
			return null;
		}

		InteractionSession session = InteractionSession.findSession(sessionIdentifier);

		if(session == null) {
			Hackontrol.LOGGER.warn("No session found for session identifier: {}", identifier);
			return null;
		}

		return session;
	}

	private static InteractionSession findSession(long sessionIdentifier) {
		for(int i = 0; i < InteractionSession.SESSION_LIST.size(); i++) {
			InteractionSession session = InteractionSession.SESSION_LIST.get(i);

			if(session.sessionIdentifier == sessionIdentifier) {
				return session;
			}
		}

		return null;
	}

	private static boolean hasCollision(long sessionIdentifier) {
		for(int i = 0; i < InteractionSession.SESSION_LIST.size(); i++) {
			if(InteractionSession.SESSION_LIST.get(i).sessionIdentifier == sessionIdentifier) {
				return true;
			}
		}

		return false;
	}

	static enum InteractionType {
		BUTTON,
		MODAL,
		STRING_SELECT_MENU;
	}
}
