package com.khopan.hackontrol.panel;

import java.util.ArrayList;
import java.util.Collections;
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
import net.dv8tion.jda.api.entities.channel.middleman.GuildChannel;
import net.dv8tion.jda.api.requests.restaction.order.CategoryOrderAction;

public class PanelManager {
	private final List<Panel> panelList;
	private final Map<Registrable<?>, List<Object>> registrableMap;
	private final Map<BiRegistrable<?, ?>, Map<Object, Object>> biRegistrableMap;

	public PanelManager() {
		this.panelList = new ArrayList<>();
		this.registrableMap = new HashMap<>();
		this.biRegistrableMap = new HashMap<>();
	}

	public void register(Panel panel) {
		if(panel == null) {
			throw new NullPointerException("Panel cannot be null");
		}

		this.panelList.add(panel);
	}

	public List<Panel> panelList() {
		return Collections.unmodifiableList(this.panelList);
	}

	public void initialize(Category category) {
		if(category == null) {
			throw new NullPointerException("Category cannot be null");
		}

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
			panel.registeration();
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

		CategoryOrderAction action = category.modifyTextChannelPositions();
		List<Integer> orderList = new ArrayList<>();

		for(GuildChannel channel : action.getCurrentOrder()) {
			long identifier = channel.getIdLong();

			for(int i = 0; i < this.panelList.size(); i++) {
				if(identifier == this.panelList.get(i).channel.getIdLong()) {
					orderList.add(i);
					break;
				}
			}
		}

		int size = orderList.size();

		for(int x = 0; x < size; x++) {
			int target = -1;

			for(int y = x; y < size; y++) {
				if(orderList.get(y) == x) {
					target = y;
					break;
				}
			}

			if(target == -1 || target == x) {
				continue;
			}

			action.selectPosition(x);
			action.swapPosition(target);
			int buffer = orderList.get(x);
			orderList.set(x, orderList.get(target));
			orderList.set(target, buffer);
		}

		action.queue();
	}

	@SuppressWarnings("unchecked")
	public <T> List<T> getRegistrable(Registrable<T> registrable) {
		if(registrable == null) {
			return List.of();
		}

		List<Object> objectList = this.registrableMap.get(registrable);

		if(objectList == null) {
			return List.of();
		}

		List<T> list = new ArrayList<>();

		for(Object object : objectList) {
			list.add((T) object);
		}

		return list;
	}

	@SuppressWarnings("unchecked")
	public <T, U> Map<T, U> getRegistrable(BiRegistrable<T, U> registrable) {
		if(registrable == null) {
			return Map.of();
		}

		Map<Object, Object> objectMap = this.biRegistrableMap.get(registrable);

		if(objectMap == null) {
			return Map.of();
		}

		Map<T, U> map = new HashMap<>();
		objectMap.forEach((key, value) -> map.put((T) key, (U) value));
		return map;
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
