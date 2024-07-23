package com.khopan.hackontrol.remote.session;

import java.awt.GridLayout;

import javax.swing.ButtonGroup;
import javax.swing.JCheckBox;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.border.TitledBorder;

import com.khopan.hackontrol.remote.component.StreamView;

public class StreamPanel extends JPanel {
	private static final long serialVersionUID = 1236103425178702868L;

	public final StreamView streamView;

	private final JCheckBox streamingCheckBox;
	private final JRadioButton sendAsFullImage;
	private final JRadioButton sendAsBoundaryDifferencesButton;
	private final JRadioButton sendAsColorDifferencesButton;
	private final JRadioButton sendAsUncompressedButton;

	public StreamPanel(int sourceWidth, int sourceHeight) {
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
		ButtonGroup sendOptionGroup = new ButtonGroup();
		this.sendAsFullImage = new JRadioButton();
		this.sendAsFullImage.setText("Send as full (compressed) image");
		streamControlPanel.add(this.sendAsFullImage);
		sendOptionGroup.add(this.sendAsFullImage);
		this.sendAsBoundaryDifferencesButton = new JRadioButton();
		this.sendAsBoundaryDifferencesButton.setText("Send as boundary differences");
		streamControlPanel.add(this.sendAsBoundaryDifferencesButton);
		sendOptionGroup.add(this.sendAsBoundaryDifferencesButton);
		this.sendAsColorDifferencesButton = new JRadioButton();
		this.sendAsColorDifferencesButton.setText("Send as color differences");
		streamControlPanel.add(this.sendAsColorDifferencesButton);
		sendOptionGroup.add(this.sendAsColorDifferencesButton);
		sendOptionGroup.setSelected(this.sendAsColorDifferencesButton.getModel(), true);
		this.sendAsUncompressedButton = new JRadioButton();
		this.sendAsUncompressedButton.setText("Send as uncompressed image");
		streamControlPanel.add(this.sendAsUncompressedButton);
		sendOptionGroup.add(this.sendAsUncompressedButton);
		belowStreamPanel.add(streamControlPanel);
		this.add(belowStreamPanel);
		this.updateCheckboxState();
	}

	private void updateCheckboxState() {
		boolean state = this.streamingCheckBox.isSelected();
		this.sendAsFullImage.setEnabled(state);
		this.sendAsBoundaryDifferencesButton.setEnabled(state);
		this.sendAsColorDifferencesButton.setEnabled(state);
		this.sendAsUncompressedButton.setEnabled(state);
	}
}
