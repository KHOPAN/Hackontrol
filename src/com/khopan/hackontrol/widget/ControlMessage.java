package com.khopan.hackontrol.widget;

import net.dv8tion.jda.api.entities.Message;

public class ControlMessage {
	public ControlMessage() {

	}

	public static ControlMessage fromMessage(Message message) {
		return null;
	}

	public static ControlMessageBuilder newBuilder() {
		return new ControlMessageBuilder();
	}

	public static class ControlMessageBuilder {
		private ControlMessageBuilder() {

		}

		public ControlMessage build() {
			return null;
		}
	}
}
