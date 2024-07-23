package com.khopan.hackontrol.remote.session;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.io.ByteArrayInputStream;
import java.util.Arrays;

import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;

public class StreamView extends Component {
	private static final long serialVersionUID = 2380631139944740419L;

	private static final int QOI_OP_RGB   = 0b11111110;
	private static final int QOI_OP_INDEX = 0b00000000;
	private static final int QOI_OP_DIFF  = 0b01000000;
	private static final int QOI_OP_LUMA  = 0b10000000;
	private static final int QOI_OP_RUN   = 0b11000000;
	private static final int OP_MASK      = 0b11000000;

	private final int sourceWidth;
	private final int sourceHeight;
	private final int[] indexTable;
	private final BufferedImage sourceImage;
	private final int[] receiveBuffer;
	private final JFrame popOutWindow;
	private final PopupComponent popupComponent;
	private final JPopupMenu popupMenu;
	private final JMenuItem popInOutItem;
	private final JCheckBoxMenuItem limitToScreenBoundsBox;
	private final JCheckBoxMenuItem lockBox;

	private volatile int width;
	private volatile int height;

	private Image image;
	private int x;
	private int y;
	private boolean pictureInPicture;

	public StreamView(int width, int height) {
		this.sourceWidth = width;
		this.sourceHeight = height;
		this.indexTable = new int[64];
		this.sourceImage = new BufferedImage(this.sourceWidth, this.sourceHeight, BufferedImage.TYPE_INT_RGB);
		this.receiveBuffer = ((DataBufferInt) this.sourceImage.getRaster().getDataBuffer()).getData();
		this.popOutWindow = new JFrame();
		this.popOutWindow.setTitle("Hackontrol Remote - Picture In Picture");
		this.popOutWindow.setUndecorated(true);
		this.popOutWindow.setLayout(new BorderLayout());
		this.popupComponent = new PopupComponent();
		this.popOutWindow.add(this.popupComponent, BorderLayout.CENTER);
		this.popOutWindow.setAlwaysOnTop(true);
		this.popupMenu = new JPopupMenu();
		this.popInOutItem = new JMenuItem("Pop Out");
		this.popInOutItem.addActionListener(Event -> {
			if(this.pictureInPicture) {
				this.popIn();
			} else {
				this.popOut();
			}
		});

		this.popupMenu.add(this.popInOutItem);
		this.lockBox = new JCheckBoxMenuItem("Lock frame");
		this.lockBox.addActionListener(Event -> this.popupComponent.listener.lock = this.lockBox.isSelected());
		this.popupMenu.add(this.lockBox);
		this.limitToScreenBoundsBox = new JCheckBoxMenuItem("Limit to screen bounds");
		this.limitToScreenBoundsBox.addActionListener(Event -> this.popupComponent.listener.limitToScreenCheckBox());
		this.limitToScreenBoundsBox.setSelected(true);
		this.popupMenu.add(this.limitToScreenBoundsBox);
		this.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent Event) {
				if(!SwingUtilities.isRightMouseButton(Event)) {
					return;
				}

				StreamView.this.popInOutItem.setText(StreamView.this.pictureInPicture ? "Pop In" : "Pop Out");
				StreamView.this.lockBox.setVisible(false);
				StreamView.this.limitToScreenBoundsBox.setVisible(false);
				StreamView.this.popupMenu.show(StreamView.this, Event.getX(), Event.getY());
			}
		});
	}

	@SuppressWarnings("deprecation")
	@Override
	public void reshape(int x, int y, int width, int height) {
		super.reshape(x, y, width, height);
		this.width = width;
		this.height = height;

		if(!this.pictureInPicture) {
			this.updateImage();
		}
	}

	@Override
	public void paint(Graphics Graphics) {
		Graphics.setColor(new Color(0x000000));
		Graphics.fillRect(0, 0, this.width, this.height);

		if(!this.pictureInPicture && this.image != null) {
			Graphics.drawImage(this.image, this.x, this.y, null);
		}
	}

	public void showWindow() {
		if(this.pictureInPicture) {
			this.popOutWindow.setVisible(true);
		}
	}

	public void hideWindow() {
		if(this.pictureInPicture) {
			this.popOutWindow.dispose();
		}
	}

	public void decode(byte[] data) {
		if(data == null || data.length < 1) {
			return;
		}

		ByteArrayInputStream stream = new ByteArrayInputStream(data);
		int flags = stream.read() & 0xFF;
		boolean boundaryDifference = (flags & 1) == 1;
		boolean colorDifference = ((flags >> 1) & 1) == 1;

		if(boundaryDifference && colorDifference) {
			for(int y = 0; y < this.sourceHeight; y++) {
				for(int x = 0; x < this.sourceWidth; x++) {
					this.receiveBuffer[y * this.sourceWidth + x] = ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF);
				}
			}

			this.updateImage();
			return;
		}

		int startX = boundaryDifference ? ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF) : 0;
		int startY = boundaryDifference ? ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF) : 0;
		int endX = boundaryDifference ? ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF) : this.sourceWidth - 1;
		int endY = boundaryDifference ? ((stream.read() & 0xFF) << 24) | ((stream.read() & 0xFF) << 16) | ((stream.read() & 0xFF) << 8) | (stream.read() & 0xFF) : this.sourceHeight - 1;
		Arrays.fill(this.indexTable, 0);
		int red = 0;
		int green = 0;
		int blue = 0;
		int run = 0;

		for(int y = startY; y <= endY; y++) {
			for(int x = startX; x <= endX; x++) {
				int pixelIndex = y * this.sourceWidth + x;

				if(run > 0) {
					this.subtract(colorDifference, pixelIndex, ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
					run--;
					continue;
				}

				int chunk = stream.read() & 0xFF;

				if(chunk == StreamView.QOI_OP_RGB) {
					red = stream.read();
					green = stream.read();
					blue = stream.read();
					this.subtract(colorDifference, pixelIndex, this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
					continue;
				}

				switch(chunk & StreamView.OP_MASK) {
				case StreamView.QOI_OP_INDEX:
					int index = chunk & 0b111111;
					red = (this.indexTable[index] >> 16) & 0xFF;
					green = (this.indexTable[index] >> 8) & 0xFF;
					blue = this.indexTable[index] & 0xFF;
					break;
				case StreamView.QOI_OP_DIFF:
					red += ((chunk >> 4) & 0b11) - 2;
					green += ((chunk >> 2) & 0b11) - 2;
					blue += (chunk & 0b11) - 2;
					break;
				case StreamView.QOI_OP_LUMA:
					int next = stream.read() & 0xFF;
					int differenceGreen = (chunk & 0b111111) - 32;
					red += differenceGreen - 8 + ((next >> 4) & 0b1111);
					green += differenceGreen;
					blue += differenceGreen - 8 + (next & 0b1111);
					break;
				case StreamView.QOI_OP_RUN:
					this.subtract(colorDifference, pixelIndex, ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
					run = (chunk & 0b111111);
					continue;
				}

				this.subtract(colorDifference, pixelIndex, this.indexTable[((red & 0xFF) * 3 + (green & 0xFF) * 5 + (blue & 0xFF) * 7 + 0xFF * 11) & 0b111111] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF));
			}
		}

		this.updateImage();
	}

	private void subtract(boolean colorDifference, int index, int color) {
		if(!colorDifference) {
			this.receiveBuffer[index] = color;
			return;
		}

		int red = ((this.receiveBuffer[index] >> 16) & 0xFF) - ((color >> 16) & 0xFF);
		int green = ((this.receiveBuffer[index] >> 8) & 0xFF) - ((color >> 8) & 0xFF);
		int blue = (this.receiveBuffer[index] & 0xFF) - (color & 0xFF);
		this.receiveBuffer[index] = ((red & 0xFF) << 16) | ((green & 0xFF) << 8) | (blue & 0xFF);
	}

	private void updateImage() {
		int width = this.pictureInPicture ? this.popupComponent.width : this.width;
		int height = this.pictureInPicture ? this.popupComponent.height : this.height;

		if(this.sourceImage == null || !this.isVisible() || width < 1 || height < 1) {
			return;
		}

		int imageWidth = this.sourceImage.getWidth();
		int imageHeight = this.sourceImage.getHeight();
		int newWidth = (int) Math.round(((double) imageWidth) / ((double) imageHeight) * ((double) height));
		int newHeight = (int) Math.round(((double) imageHeight) / ((double) imageWidth) * ((double) width));

		if(newWidth < width) {
			newHeight = height;
			this.x = (int) Math.round((((double) width) - ((double) newWidth)) * 0.5d);
			this.y = 0;
		} else {
			newWidth = width;
			this.x = 0;
			this.y = (int) Math.round((((double) height) - ((double) newHeight)) * 0.5d);
		}

		this.image = this.sourceImage.getScaledInstance(newWidth, newHeight, Image.SCALE_FAST);

		if(this.pictureInPicture) {
			this.popupComponent.repaint();
		} else {
			this.repaint();
		}
	}

	private void popOut() {
		Point location = this.getLocationOnScreen();
		this.popOutWindow.setBounds(location.x, location.y, this.width, this.height);
		this.pictureInPicture = true;
		this.updateImage();
		this.repaint();
		this.popOutWindow.setVisible(true);
	}

	private void popIn() {
		this.pictureInPicture = false;
		this.updateImage();
		this.popOutWindow.dispose();
	}

	private class PopupComponent extends Component {
		private static final long serialVersionUID = 59381208011157379L;

		private final PopupListener listener;

		private int width;
		private int height;

		private PopupComponent() {
			this.listener = new PopupListener();
			this.addMouseListener(this.listener);
			this.addMouseMotionListener(this.listener);
		}

		@SuppressWarnings("deprecation")
		@Override
		public void reshape(int x, int y, int width, int height) {
			super.reshape(x, y, width, height);
			this.width = width;
			this.height = height;

			if(StreamView.this.pictureInPicture) {
				StreamView.this.updateImage();
			}
		}

		@Override
		public void paint(Graphics Graphics) {
			Graphics.setColor(new Color(0x000000));
			Graphics.fillRect(0, 0, this.width, this.height);

			if(StreamView.this.pictureInPicture && StreamView.this.image != null) {
				Graphics.drawImage(StreamView.this.image, StreamView.this.x, StreamView.this.y, null);
			}
		}

		private class PopupListener extends MouseAdapter {
			private final int border;
			private final int minimumSize;

			private int pressedX;
			private int pressedY;
			private Rectangle bounds;
			private int cursor;
			private boolean limitScreen;
			private boolean lock;

			private PopupListener() {
				this.border = 10;
				this.minimumSize = this.border * 2;
				this.limitScreen = true;
			}

			private void limitToScreenCheckBox() {
				this.limitScreen = StreamView.this.limitToScreenBoundsBox.isSelected();
				Rectangle bounds = StreamView.this.popOutWindow.getBounds();
				this.limitToScreenBounds(bounds);
				StreamView.this.popOutWindow.setBounds(bounds);
			}

			@Override
			public void mousePressed(MouseEvent Event) {
				this.pressedX = Event.getX();
				this.pressedY = Event.getY();
				this.bounds = StreamView.this.popOutWindow.getBounds();
			}

			@Override
			public void mouseDragged(MouseEvent Event) {
				if(this.lock) {
					return;
				}

				Point point = Event.getLocationOnScreen();
				Rectangle bounds = StreamView.this.popOutWindow.getBounds();

				if(this.cursor == Cursor.N_RESIZE_CURSOR || this.cursor == Cursor.NW_RESIZE_CURSOR || this.cursor == Cursor.NE_RESIZE_CURSOR) {
					bounds.height = this.bounds.y + this.bounds.height - bounds.y;

					if(this.bounds.y + this.bounds.height - point.y + this.pressedY >= this.minimumSize) {
						bounds.y = point.y - this.pressedY;
					}
				}

				if(this.cursor == Cursor.E_RESIZE_CURSOR || this.cursor == Cursor.NE_RESIZE_CURSOR || this.cursor == Cursor.SE_RESIZE_CURSOR) {
					bounds.width = point.x - this.bounds.x + this.bounds.width - this.pressedX;
				}

				if(this.cursor == Cursor.S_RESIZE_CURSOR || this.cursor == Cursor.SE_RESIZE_CURSOR || this.cursor == Cursor.SW_RESIZE_CURSOR) {
					bounds.height = point.y - this.bounds.y + this.bounds.height - this.pressedY;
				}

				if(this.cursor == Cursor.W_RESIZE_CURSOR || this.cursor == Cursor.SW_RESIZE_CURSOR || this.cursor == Cursor.NW_RESIZE_CURSOR) {
					bounds.width = this.bounds.x + this.bounds.width - bounds.x;

					if(this.bounds.x + this.bounds.width - point.x + this.pressedX >= this.minimumSize) {
						bounds.x = point.x - this.pressedX;
					}
				}

				if(this.cursor == Cursor.DEFAULT_CURSOR) {
					bounds.x = point.x - this.pressedX;
					bounds.y = point.y - this.pressedY;
				}

				bounds.width = Math.max(bounds.width, this.minimumSize);
				bounds.height = Math.max(bounds.height, this.minimumSize);
				this.limitToScreenBounds(bounds);
				StreamView.this.popOutWindow.setBounds(bounds);
			}

			@Override
			public void mouseMoved(MouseEvent Event) {
				if(this.lock) {
					return;
				}

				int x = Event.getX();
				int y = Event.getY();
				int width = PopupComponent.this.getWidth();
				int height = PopupComponent.this.getHeight();
				boolean north = y >= 0 && y <= this.border;
				boolean east = x >= width - this.border && x < width;
				boolean south = y >= height - this.border && y < height;
				boolean west = x >= 0 && x <= this.border;
				this.cursor = north ? west ? Cursor.NW_RESIZE_CURSOR : east ? Cursor.NE_RESIZE_CURSOR : Cursor.N_RESIZE_CURSOR : south ? west ? Cursor.SW_RESIZE_CURSOR : east ? Cursor.SE_RESIZE_CURSOR : Cursor.S_RESIZE_CURSOR : west ? Cursor.W_RESIZE_CURSOR : east ? Cursor.E_RESIZE_CURSOR : Cursor.DEFAULT_CURSOR;
				PopupComponent.this.setCursor(Cursor.getPredefinedCursor(this.cursor));
			}

			@Override
			public void mouseClicked(MouseEvent Event) {
				if(!SwingUtilities.isRightMouseButton(Event)) {
					return;
				}

				StreamView.this.popInOutItem.setText("Pop In");
				StreamView.this.lockBox.setVisible(true);
				StreamView.this.limitToScreenBoundsBox.setVisible(true);
				StreamView.this.popupMenu.show(PopupComponent.this, Event.getX(), Event.getY());
			}

			private void limitToScreenBounds(Rectangle bounds) {
				if(!this.limitScreen) {
					return;
				}

				bounds.x = Math.min(Math.max(bounds.x, 0), StreamView.this.sourceWidth - bounds.width);
				bounds.y = Math.min(Math.max(bounds.y, 0), StreamView.this.sourceHeight - bounds.height);
			}
		}
	}
}
