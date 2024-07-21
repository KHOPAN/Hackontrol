package com.khopan.hackontrol.remote.component;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;

public class StreamView extends Component {
	private static final long serialVersionUID = 5488818679972601274L;

	private BufferedImage sourceImage;
	private Image image;
	private int x;
	private int y;
	private int width;
	private int height;

	public void setImage(BufferedImage image) {
		this.sourceImage = image;
		this.update();
	}

	@SuppressWarnings("deprecation")
	@Override
	public void reshape(int x, int y, int width, int height) {
		super.reshape(x, y, width, height);
		this.width = width;
		this.height = height;
		this.update();
	}

	private void update() {
		if(this.sourceImage == null) {
			return;
		}

		int imageWidth = this.sourceImage.getWidth();
		int imageHeight = this.sourceImage.getHeight();
		int newWidth = (int) Math.round(((double) imageWidth) / ((double) imageHeight) * ((double) this.height));
		int newHeight = (int) Math.round(((double) imageHeight) / ((double) imageWidth) * ((double) this.width));

		if(newWidth < this.width) {
			newHeight = this.height;
			this.x = (int) Math.round((((double) this.width) - ((double) newWidth)) * 0.5d);
			this.y = 0;
		} else {
			newWidth = this.width;
			this.x = 0;
			this.y = (int) Math.round((((double) this.height) - ((double) newHeight)) * 0.5d);
		}

		this.image = this.sourceImage.getScaledInstance(newWidth, newHeight, Image.SCALE_FAST);
		this.repaint();
	}

	@Override
	public void paint(Graphics Graphics) {
		Graphics.setColor(new Color(0x000000));
		Graphics.fillRect(0, 0, this.width, this.height);

		if(this.image != null) {
			Graphics.drawImage(this.image, this.x, this.y, null);
		}
	}
}
