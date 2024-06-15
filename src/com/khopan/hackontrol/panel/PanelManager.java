package com.khopan.hackontrol.panel;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.khopan.hackontrol.registry.BiRegistrable;
import com.khopan.hackontrol.registry.Registrable;
import com.khopan.hackontrol.security.SecurityManager;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public class PanelManager {
	private final List<Panel> panelList;
	private final Map<Registrable<?>, List<Object>> registrableMap;
	private final Map<BiRegistrable<?, ?>, Map<Object, Object>> biRegistrableMap;

	public PanelManager() {
		this.panelList = new ArrayList<>();
		this.registrableMap = new HashMap<>();
		this.biRegistrableMap = new HashMap<>();
	}

	public void add(Panel panel) {
		this.panelList.add(panel);
	}

	public void initialize(Category category) {
		for(Panel panel : this.panelList) {
			panel.manager = this;
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
					this.processControlWidget(channel, widgets);
				} else {
					ControlWidget.send(channel, widgets);
				}
			}

			panel.initialize();
		}
	}

	<T> void register(Registrable<T> registrable, T value) {
		if(this.registrableMap.containsKey(registrable)) {
			this.registrableMap.get(registrable).add(value);
			return;
		}

		List<Object> list = new ArrayList<>();
		list.add(value);
		this.registrableMap.put(registrable, list);
	}

	<T, U> void register(BiRegistrable<T, U> registrable, T key, U value) {
		if(this.biRegistrableMap.containsKey(registrable)) {
			this.biRegistrableMap.get(registrable).put(key, value);
			return;
		}

		Map<Object, Object> map = new HashMap<>();
		map.put(key, value);
		this.biRegistrableMap.put(registrable, map);
	}

	private void processControlWidget(TextChannel channel, ControlWidget[] widgets) {
		List<ControlWidget> widgetList = new ArrayList<>();
		widgetList.addAll(List.of(widgets));
		List<Message> list = MessageHistory.getHistoryFromBeginning(channel).complete().getRetrievedHistory();

		for(Message message : list) {
			ControlWidget controlWidget = ControlWidget.fromMessage(message);

			for(ControlWidget widget : widgetList) {
				if(widget.equals(controlWidget)) {
					widgetList.remove(widget);
					break;
				}
			}
		}

		for(ControlWidget widget : widgetList) {
			widget.send(channel);
		}
	}
}
