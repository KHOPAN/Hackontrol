package com.khopan.hackontrol.widget;

import java.util.ArrayList;
import java.util.List;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.LayoutComponent;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class ControlWidget {
	private final String text;
	private final List<LayoutComponent> list;

	private ControlWidget(String text, List<LayoutComponent> list) {
		this.text = text;
		this.list = list;
	}

	public void send(MessageChannel channel) {
		if(channel == null) {
			throw new NullPointerException("Channel cannot be null");
		}

		MessageCreateBuilder builder = new MessageCreateBuilder();

		if(this.text != null) {
			builder.setContent(this.text);
		}

		if(!this.list.isEmpty()) {
			builder.addComponents(this.list);
		}

		channel.sendMessage(builder.build()).queue();
	}

	@Override
	public boolean equals(Object another) {
		if(another == null) {
			return false;
		}

		if(!(another instanceof ControlWidget)) {
			return false;
		}

		ControlWidget widget = (ControlWidget) another;

		if(!this.text.equals(widget.text)) {
			return false;
		}

		int size = this.list.size();

		if(size != widget.list.size()) {
			return false;
		}

		if(size == 0) {
			return true;
		}

		for(int i = 0; i < size; i++) {
			if(!this.list.get(i).equals(widget.list.get(i))) {
				return false;
			}
		}

		return true;
	}

	public static ControlWidget fromMessage(Message message) {
		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		return new ControlWidget(message.getContentRaw(), message.getComponents());
	}

	public static ControlWidgetBuilder newBuilder() {
		return new ControlWidgetBuilder();
	}

	public static void send(MessageChannel channel, ControlWidget[] widgets) {
		if(channel == null) {
			throw new NullPointerException("Channel cannot be null");
		}

		if(widgets == null) {
			throw new NullPointerException("Widgets cannot be null");
		}

		for(ControlWidget widget : widgets) {
			widget.send(channel);
		}
	}

	public static class ControlWidgetBuilder {
		private final List<LayoutComponent> components;

		private String text;

		private ControlWidgetBuilder() {
			this.components = new ArrayList<>();
			this.text = "";
		}

		public ControlWidgetBuilder text(String text) {
			this.text = text == null ? "" : text;
			return this;
		}

		public ControlWidgetBuilder actionRow(ItemComponent... components) {
			if(components == null || components.length == 0) {
				return this;
			}

			return this.layoutComponent(ActionRow.of(components));
		}

		public ControlWidgetBuilder layoutComponent(LayoutComponent... components) {
			if(components == null || components.length == 0) {
				return this;
			}

			for(LayoutComponent component : components) {
				if(component != null) {
					this.components.add(component);
				}
			}

			return this;
		}

		public ControlWidget build() {
			return new ControlWidget(this.text, this.components);
		}
	}
}
