package pl.pachciarek.ffmpeg_android_example;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.Button;
import android.view.View.OnClickListener;
import android.view.View;
import android.widget.EditText;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView sv = (SurfaceView) findViewById(R.id.surfaceView);
        SurfaceHolder sh = sv.getHolder();
        final Surface s = sh.getSurface();

        final EditText filePathInput = ((EditText) findViewById(R.id.filePath));
        filePathInput.setText(Environment.getExternalStorageDirectory().getAbsolutePath() + "/test.mp4");

        ((Button) findViewById(R.id.buttonStart)).setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                FFmpegTest.getInstance().setFilePath(filePathInput.getText().toString());
                FFmpegTest.getInstance().setSurface(s);

                FFmpegTest.getInstance().startRendering();
            }
        });

        ((Button) findViewById(R.id.buttonStop)).setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                FFmpegTest.getInstance().endRendering();
            }
        });
    }
}
