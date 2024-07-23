package com.khopan.hackontrol.remote.session;

import java.awt.GridLayout;
import java.io.OutputStream;

import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.border.TitledBorder;

import com.khopan.hackontrol.remote.component.StreamView;
import com.khopan.hackontrol.remote.network.Packet;

public class StreamPanel extends JPanel {
	private static final long serialVersionUID = 1236103425178702868L;

	public final StreamView streamView;

	private final OutputStream stream;
	private final JCheckBox streamingCheckBox;
	private final ButtonGroup sendOptionGroup;
	private final JRadioButton sendAsFullImage;
	private final JRadioButton sendAsBoundaryDifferencesButton;
	private final JRadioButton sendAsColorDifferencesButton;
	private final JRadioButton sendAsUncompressedButton;

	private int flags;

	public StreamPanel(OutputStream stream, int sourceWidth, int sourceHeight) {
		this.stream = stream;
		this.setBorder(new TitledBorder("Stream"));
		this.setLayout(new GridLayout(2, 1));
		this.streamView = new StreamView(sourceWidth, sourceHeight);
		this.add(this.streamView);
		JPanel belowStreamPanel = new JPanel();
		belowStreamPanel.setLayout(new GridLayout(2, 1));
		belowStreamPanel.add(new JPanel());
		JPanel streamControlPanel = new JPanel();
		streamControlPanel.setBorder(new TitledBorder("Stream Control"));
		streamControlPanel.setLayout(new GridLayout(5, 1));
		this.streamingCheckBox = new JCheckBox();
		this.streamingCheckBox.setText("Enable Streaming");
		this.streamingCheckBox.addActionListener(Event -> this.updateCheckboxState());
		streamControlPanel.add(this.streamingCheckBox);
		this.sendOptionGroup = new ButtonGroup();
		this.sendAsFullImage = new JRadioButton();
		this.sendAsFullImage.setText("Send as full (compressed) image");
		this.sendAsFullImage.addActionListener(Event -> this.updateCheckboxState());
		streamControlPanel.add(this.sendAsFullImage);
		this.sendOptionGroup.add(this.sendAsFullImage);
		this.sendAsBoundaryDifferencesButton = new JRadioButton();
		this.sendAsBoundaryDifferencesButton.setText("Send as boundary differences");
		this.sendAsBoundaryDifferencesButton.addActionListener(Event -> this.updateCheckboxState());
		streamControlPanel.add(this.sendAsBoundaryDifferencesButton);
		this.sendOptionGroup.add(this.sendAsBoundaryDifferencesButton);
		this.sendAsColorDifferencesButton = new JRadioButton();
		this.sendAsColorDifferencesButton.setText("Send as color differences");
		this.sendAsColorDifferencesButton.addActionListener(Event -> this.updateCheckboxState());
		streamControlPanel.add(this.sendAsColorDifferencesButton);
		this.sendOptionGroup.add(this.sendAsColorDifferencesButton);
		this.sendOptionGroup.setSelected(this.sendAsColorDifferencesButton.getModel(), true);
		this.sendAsUncompressedButton = new JRadioButton();
		this.sendAsUncompressedButton.setText("Send as uncompressed image");
		this.sendAsUncompressedButton.addActionListener(Event -> this.updateCheckboxState());
		streamControlPanel.add(this.sendAsUncompressedButton);
		this.sendOptionGroup.add(this.sendAsUncompressedButton);
		belowStreamPanel.add(streamControlPanel);
		this.add(belowStreamPanel);
		this.flags = 0;
		this.updateCheckboxState();
	}

	private void updateCheckboxState() {
		boolean state = this.streamingCheckBox.isSelected();
		this.sendAsFullImage.setEnabled(state);
		this.sendAsBoundaryDifferencesButton.setEnabled(state);
		this.sendAsColorDifferencesButton.setEnabled(state);
		this.sendAsUncompressedButton.setEnabled(state);
		int flags = 0b00;

		if(this.sendAsFullImage.isSelected()) {
			flags = 0b00;
		} else if(this.sendAsBoundaryDifferencesButton.isSelected()) {
			flags = 0b01;
		} else if(this.sendAsColorDifferencesButton.isSelected()) {
			flags = 0b10;
		} else if(this.sendAsUncompressedButton.isSelected()) {
			flags = 0b11;
		}

		flags = ((flags & 0b11) << 1) | (state ? 1 : 0);

		if(this.flags == flags) {
			return;
		}

		this.flags = flags;

		try {
			Packet.writePacket(this.stream, Packet.of(new byte[] {(byte) this.flags}, Packet.PACKET_TYPE_STREAM_FRAME));
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}
	}
}
