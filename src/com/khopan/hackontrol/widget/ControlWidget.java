package com.khopan.hackontrol.widget;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.LayoutComponent;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ControlWidget {
	private final MessageCreateData data;

	private ControlWidget(MessageCreateData data) {
		this.data = data;
	}

	public void send(MessageChannel channel) {
		channel.sendMessage(this.data).queue();
	}

	public static ControlWidget fromMessage(Message message) {
		return new ControlWidget(MessageCreateData.fromMessage(message));
	}

	public static ControlWidgetBuilder newBuilder() {
		return new ControlWidgetBuilder();
	}

	public static void send(MessageChannel channel, ControlWidget[] widgets) {
		for(ControlWidget widget : widgets) {
			widget.send(channel);
		}
	}

	public static class ControlWidgetBuilder {
		private final MessageCreateBuilder builder;

		private ControlWidgetBuilder() {
			this.builder = new MessageCreateBuilder();
		}

		public ControlWidgetBuilder text(String text) {
			this.builder.addContent(text);
			return this;
		}

		public ControlWidgetBuilder actionRow(ItemComponent... components) {
			this.builder.addActionRow(components);
			return this;
		}

		public ControlWidgetBuilder layoutComponent(LayoutComponent... components) {
			this.builder.addComponents(components);
			return this;
		}

		public ControlWidget build() {
			return new ControlWidget(this.builder.build());
		}
	}
}
