package com.parallelcomp.socketconnection;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    EditText addressText;
    EditText portText;
    EditText messageText;
    TextView responseText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        addressText = findViewById(R.id.edit_text_ip_address);
        portText = findViewById(R.id.edit_text_port);
        messageText = findViewById(R.id.edit_text_query);
        responseText = findViewById(R.id.text_response);

        Button sendButton = findViewById(R.id.send);
        sendButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        Log.i("button", "clicked");

        sendMessage(
                addressText.getText().toString(),
                Integer.parseInt(portText.getText().toString()),
                messageText.getText().toString()
        );
    }

    private void sendMessage(final String address, final int port, final String message) {

        Thread senderThread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    Socket sock = new Socket(address, port);

                    PrintWriter output = new PrintWriter(sock.getOutputStream());
                    output.print(message);
                    output.close();
                    Log.i("sender", "message sent");

                    BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));

                    sock.close();

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });

        senderThread.start();
    }
}
