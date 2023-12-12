package com.khopan.hackontrol.command;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.source.CommandSource;
import com.khopan.win32.Win32;
import com.khopan.win32.struct.holder.WSADATA;
import com.khopan.win32.utils.MutableInteger;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;
import com.mojang.brigadier.context.CommandContext;

public class WakeCommand implements Command {
	/*@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("wake").requires(source -> source.isSelected()).then(RequiredArgumentBuilder.<CommandSource, String>argument("ip", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String ip = StringArgumentType.getString(context, "ip");
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("all").executes(context -> {
			return 1;
		})));
	}

	private static native void wakeAddress(CommandSource source, int ipAddress);
	private static native void wakeAll(CommandSource source);*/

	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("wake").requires(source -> source.isSelected()).then(RequiredArgumentBuilder.<CommandSource, String>argument("ip", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String ip = StringArgumentType.getString(context, "ip");
			WSADATA data = new WSADATA();
			long result = Win32.WSAStartup(0x0202, data);

			if(result != 0) {
				source.message("Error: " + Win32.FormatErrorMessage(result).trim());
				return -1;
			}

			InetAddress address;

			try {
				address = InetAddress.getByName(ip);
			} catch(Throwable Errors) {
				source.message("Error: " + Errors.toString());
				return -1;
			}

			byte[] addressByte = address.getAddress();
			byte[] macResult = new byte[6];
			result = Win32.SendARP((addressByte[0] & 0xFF) | ((addressByte[1] & 0xFF) << 8) | ((addressByte[2] & 0xFF) << 16) | ((addressByte[3] & 0xFF) << 24), 0, macResult, new MutableInteger(macResult.length));

			if(result != 0) {
				source.message("Error: " + Win32.FormatErrorMessage(result).trim());
				return -1;
			}

			byte[] bytes = new byte[6 + 16 * macResult.length];

			for(int i = 0; i < 6; i++) {
				bytes[i] = (byte) 0xFF;
			}

			for(int i = 6; i < bytes.length; i += macResult.length) {
				System.arraycopy(macResult, 0, bytes, i, macResult.length);
			}

			try {
				DatagramSocket socket = new DatagramSocket();
				DatagramPacket packet = new DatagramPacket(bytes, bytes.length, address, 9);
				socket.send(packet);
				socket.close();
			} catch(Throwable Errors) {
				source.message("Error: " + Errors.toString());
				return -1;
			}

			result = Win32.WSACleanup();

			if(result != 0) {
				source.message("Error: " + Win32.FormatErrorMessage(result).trim());
				return -1;
			}

			source.message("WOL packet sent");
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("all").executes(context -> {
			return this.wakeAll(context, "192.168.1.");
		}).then(RequiredArgumentBuilder.<CommandSource, String>argument("prefix", StringArgumentType.string()).executes(context -> {
			return this.wakeAll(context, StringArgumentType.getString(context, "prefix"));
		}))));
	}

	private int wakeAll(CommandContext<CommandSource> context, String prefix) {
		CommandSource source = context.getSource();
		WSADATA data = new WSADATA();
		long result = Win32.WSAStartup(0x0202, data);

		if(result != 0) {
			source.message("Error: " + Win32.FormatErrorMessage(result).trim());
			return -1;
		}

		List<Thread> threadList = new ArrayList<>();
		List<DatagramPacket> packetList = new ArrayList<>();

		for(int i = 1; i <= 255; i++) {
			int t = i;
			Thread thread = new Thread(() -> {
				byte[] macAddress = new byte[6];

				if(Win32.SendARP(0x01A8C0 | ((t & 0xFF) << 24), 0, macAddress, new MutableInteger(macAddress.length)) != 0) {
					return;
				}

				byte[] magicBytes = new byte[6 + 16 * macAddress.length];

				for(int x = 0; x < 6; x++) {
					magicBytes[x] = (byte) 0xFF;
				}

				for(int x = 6; x < magicBytes.length; x += macAddress.length) {
					System.arraycopy(macAddress, 0, magicBytes, x, macAddress.length);
				}

				InetAddress address;

				try {
					address = InetAddress.getByName(prefix + t);
				} catch(Throwable ignored) {
					return;
				}

				DatagramPacket packet = new DatagramPacket(magicBytes, magicBytes.length, address, 9);
				packetList.add(packet);
			});

			threadList.add(thread);
			thread.start();
		}

		for(Thread thread : threadList) {
			try {
				thread.join();
			} catch(Throwable ignored) {

			}
		}

		if(packetList.isEmpty()) {
			source.message("No connected device found");
			return 1;
		}

		try {
			DatagramSocket socket = new DatagramSocket();

			for(DatagramPacket packet : packetList) {
				socket.send(packet);
			}

			socket.close();
		} catch(Throwable Errors) {
			source.message("Error: " + Errors.toString());
			return -1;
		}

		result = Win32.WSACleanup();

		if(result != 0) {
			source.message("Error: " + Win32.FormatErrorMessage(result).trim());
			return -1;
		}

		int size = packetList.size();
		source.message("WOL packet" + (size == 1 ? "" : "s") + " sent (" + size + " packet" + (size == 1 ? ")" : "s)"));
		return 1;
	}
}
