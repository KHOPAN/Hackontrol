package com.khopan.hackontrol.widget;

import net.dv8tion.jda.api.entities.Message;

public class ControlWidget {
	public ControlWidget() {

	}

	public static ControlWidget fromMessage(Message message) {
		return null;
	}

	public static ControlWidgetBuilder newBuilder() {
		return new ControlWidgetBuilder();
	}

	public static class ControlWidgetBuilder {
		private ControlWidgetBuilder() {}

		public ControlWidget build() {
			return null;
		}
	}
}
