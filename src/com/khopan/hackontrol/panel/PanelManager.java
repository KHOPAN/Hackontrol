package com.khopan.hackontrol.panel;

import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.security.SecurityManager;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public class PanelManager {
	private final List<Panel> list;

	public PanelManager() {
		this.list = new ArrayList<>();
	}

	public void add(Panel panel) {
		this.list.add(panel);
	}

	public void initialize(Category category) {
		for(Panel panel : this.list) {
			String name = panel.panelName();
			TextChannel channel = null;

			for(TextChannel textChannel : category.getTextChannels()) {
				if(textChannel.getName().equalsIgnoreCase(name)) {
					channel = textChannel;
					break;
				}
			}

			boolean hasChannel = channel != null;

			if(!hasChannel) {
				channel = category.createTextChannel(name).complete();
			}

			SecurityManager.configureViewPermission(channel.getManager());
			panel.channel = channel;
			ControlWidget[] widgets = panel.controlWidget();

			if(widgets != null && widgets.length > 0) {
				if(hasChannel) {					
					this.processControlWidget(panel, widgets);
				} else {
					ControlWidget.send(channel, widgets);
				}
			}

			panel.initialize();
		}
	}

	private void processControlWidget(Panel panel, ControlWidget[] widgets) {

	}
}
