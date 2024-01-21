package com.khopan.hackontrol;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.image.BufferedImage;

public class MessageDisplay {
	private static final Cursor BLANK_CURSOR = Toolkit.getDefaultToolkit().createCustomCursor(new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB), new Point(0, 0), "blank");

	private MessageDisplay(String message) {
		Window frame = new Window(null);
		frame.setLayout(new BorderLayout());
		frame.setAlwaysOnTop(true);
		frame.add(new Panel(message), BorderLayout.CENTER);
		frame.setSize(Toolkit.getDefaultToolkit().getScreenSize());
		frame.setCursor(MessageDisplay.BLANK_CURSOR);
		frame.setVisible(true);
		new Thread(() -> {
			try {
				Thread.sleep(5000);
				frame.dispose();
			} catch(Throwable ignored) {

			}
		}).start();
	}

	public static void display(String message) {
		new MessageDisplay(message);
	}

	private static class Panel extends Component {
		private static final long serialVersionUID = 6827310677052364511L;

		private final String message;

		private int width;
		private int height;

		private Panel(String message) {
			this.message = message;
		}

		@SuppressWarnings("deprecation")
		@Override
		public void reshape(int x, int y, int width, int height) {
			super.reshape(x, y, width, height);
			this.width = width;
			this.height = height;
		}

		@Override
		public void paint(Graphics Graphics) {
			Graphics2D Graphics2D = (Graphics2D) Graphics;
			Graphics2D.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
			Graphics2D.setColor(new Color(0xFFFFFF));
			Graphics2D.fillRect(0, 0, this.width, this.height);
			Graphics2D.setColor(new Color(0x000000));
			Graphics2D.setFont(this.getBestSize(this.message, Graphics2D, (int) Math.round(((double) this.width) * 0.85d)));
			FontMetrics metrics = Graphics2D.getFontMetrics();
			int x = (int) Math.round((((double) this.width) - ((double) metrics.stringWidth(this.message))) * 0.5d);
			int y = (int) Math.round(((double) metrics.getAscent()) * 0.5d - ((double) metrics.getDescent()) * 0.5d + ((double) this.height) * 0.5d);
			Graphics2D.drawString(this.message, x, y);
			Graphics2D.dispose();
		}

		private Font getBestSize(String text, Graphics Graphics, int width) {
			int size = 0;
			Font font;

			while(Graphics.getFontMetrics(font = new Font("Tahoma", Font.BOLD, size)).stringWidth(text) < width) {
				size++;
			}

			return font;
		}
	}
}
