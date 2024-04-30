package com.example.project.tcpip;

import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ProgressBar;
import javafx.scene.paint.Paint;
import javafx.scene.shape.Circle;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ResourceBundle;

public class AppMainController implements Initializable {
    public Label label_temperature;
    public ProgressBar progressbar_temperature;
    public Circle red_led;
    public Circle blue_led;
    public Button button_red_led;
    public Button button_blue_led;
    public ProgressBar progressbar_humidity;
    public Label label_humidity;

    private boolean state_of_button_red_led;
    private boolean state_of_button_blue_led;
    private final Socket socket;

    public AppMainController()
    {
        this.state_of_button_red_led = false;
        this.state_of_button_blue_led = false;
        this.socket = new Socket();
    }

    public void buttonOnClickedRedLED(ActionEvent actionEvent) {
        if(this.socket.isConnected()){
            this.state_of_button_red_led ^= true;
            if(state_of_button_red_led) {
                this.red_led.setFill(Paint.valueOf("red"));

                byte[]bytes_data = ("RED_LED_ON\n").getBytes(StandardCharsets.UTF_8);
                try {
                    final var output_stream = this.socket.getOutputStream();
                    output_stream.write(bytes_data);
                    output_stream.flush();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            } else {
                this.red_led.setFill(Paint.valueOf("black"));

                byte[]bytes_data = ("RED_LED_OFF\n").getBytes(StandardCharsets.UTF_8);
                try {
                    final var output_stream = this.socket.getOutputStream();
                    output_stream.write(bytes_data);
                    output_stream.flush();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        }
    }

    public void buttonOnClickedBlueLED(ActionEvent actionEvent) {
        if(this.socket.isConnected()){
            this.state_of_button_blue_led ^= true;
            if(state_of_button_blue_led) {
                this.blue_led.setFill(Paint.valueOf("blue"));

                byte[]bytes_data = ("BLUE_LED_ON\n").getBytes(StandardCharsets.UTF_8);
                try {
                    final var output_stream = this.socket.getOutputStream();
                    output_stream.write(bytes_data);
                    output_stream.flush();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            } else {
                this.blue_led.setFill(Paint.valueOf("black"));

                byte[]bytes_data = ("BLUE_LED_OFF\n").getBytes(StandardCharsets.UTF_8);
                try {
                    final var output_stream = this.socket.getOutputStream();
                    output_stream.write(bytes_data);
                    output_stream.flush();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        }
    }



    @Override
    public void initialize(URL url, ResourceBundle resourceBundle) {
        try {
            this.socket.connect(new InetSocketAddress("192.168.0.8", Integer.parseInt("9999")));
            this.received_data_from_server(new ActionEvent());
        } catch (IOException e) {
//            throw new RuntimeException(e);
            System.out.printf("%s\r\n", e.getMessage());
        }
    }

    private void received_data_from_server(ActionEvent event){
        Thread thread_of_receiving = new Thread(()->{
            while(true)
            {
                try {
                    final InputStream inputStream = this.socket.getInputStream();
                    byte[] bytes_data = new byte[512];
                    final int read_byte_count = inputStream.read(bytes_data);
                    final String serial_input_data =
                            new String(bytes_data, 0, read_byte_count, StandardCharsets.UTF_8);
                    final String[] parsings_data = serial_input_data.split(",");
                    if(parsings_data.length != 2) continue;
                    final double temperature = Double.parseDouble(parsings_data[0]);
                    final double humidity = Double.parseDouble(parsings_data[1]);
                    final double changed_temperature = change_progress_value(temperature, 0.0, 40.0, 0.0, 1.0);
                    final double changed_humidity = change_progress_value(humidity,0.0, 100.0, 0.0, 1.0);
                    System.out.printf("%s\r\n", temperature);
                    System.out.printf("%s\r\n", humidity);
                    Platform.runLater(()->{
                        progressbar_temperature.setProgress(changed_temperature);
                        progressbar_humidity.setProgress(changed_humidity);
                        label_temperature.setText(parsings_data[0]);
                        label_humidity.setText(parsings_data[1]);
                    });
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        });
        thread_of_receiving.start();
    }

    private double change_progress_value(double x, double in_min,
                                         double in_max, double out_min, double out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
}
