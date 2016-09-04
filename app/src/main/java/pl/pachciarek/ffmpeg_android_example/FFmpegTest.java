package pl.pachciarek.ffmpeg_android_example;

import android.os.AsyncTask;
import android.util.Log;
import android.view.Surface;

/**
 * Created by Jakub on 04.09.2016.
 */
public class FFmpegTest {
    private static final String TAG = FFmpegTest.class.getSimpleName();

    private static class ExecuteTask extends AsyncTask<Void, Void, Void> {

        private final FFmpegTest player;

        public ExecuteTask(FFmpegTest player) {
            this.player = player;
        }

        @Override
        protected Void doInBackground(Void... params) {
            int resultCode = this.player.startNativeRendering();

            Log.v(TAG, "Render task result code: " + new Integer(resultCode).toString());

            return null;
        }

    }

    static {
        System.loadLibrary("ffmpeg-test");
    }

    private static FFmpegTest _instance = null;

    public static FFmpegTest getInstance() {
        if (_instance == null) {
            _instance = new FFmpegTest();
        }

        return _instance;
    }

    private FFmpegTest() {}

    private ExecuteTask task = null;

    public void startRendering() {
        if (task != null && task.getStatus() != AsyncTask.Status.FINISHED) {
            return;
        }

        task = new ExecuteTask(this);
        task.execute();
    }

    public void endRendering() {
        this.endNativeRendering();
    }

    public native void setSurface(Surface view);
    public native void setFilePath(String url);
    private native void endNativeRendering();
    private native int startNativeRendering();
}
