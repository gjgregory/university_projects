package android.drunkentiger.crittercap;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.File;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;


/*
 * this is the main activity of the app where the user can start new, view, and delete discoveries
 */
public class CaptureActivity extends AppCompatActivity {
    File picture;
    String filename;
    Animation buttonPress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_capture);

        buttonPress = AnimationUtils.loadAnimation(this, R.anim.button_press);
        //initialize spinner of animal names
        Spinner nameSpinner = (Spinner) findViewById(R.id.nameSpinner);
        assert nameSpinner != null;
        nameSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                updateAnimalInfo();
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub
            }
        });

        reloadSpinner();

        // check for camera permissions if not granted yet
        if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{android.Manifest.permission.CAMERA}, 0);
        }
    }

    /* updateAnimalInfo()
     *
     * this function updates the image/text views in relation to the selected name
     */
    public void updateAnimalInfo(){
        ImageView photo = (ImageView) findViewById(R.id.imageView);
        TextView typeText = (TextView) findViewById(R.id.typeText);
        TextView colorText = (TextView) findViewById(R.id.colorText);
        TextView sizeText = (TextView) findViewById(R.id.sizeText);
        TextView traitsText = (TextView) findViewById(R.id.traitsText);
        Spinner nameSpinner = (Spinner) findViewById(R.id.nameSpinner);

        Discoveries discoveries = new Discoveries(this, null, null, 1);
        Animal animal = discoveries.getAnimal(nameSpinner.getSelectedItem().toString());

        typeText.setText(animal.getCategory());
        colorText.setText(animal.getColor());
        sizeText.setText(animal.getSize());
        traitsText.setText(animal.getTraits());
        File file = new File(animal.getFilename());
        Uri uri = Uri.fromFile(file);
        photo.setImageURI(uri);

    }


    /* newAnimal()
     *
     * this function is triggered when the user presses the "NEW" button. They are brought to the
     * camera app and then provided fields to enter a new animal into the database.
     */
    public void newAnimal(View view){
        view.startAnimation(buttonPress);
        // wait until animation is done to continue...
        view.postDelayed(new Runnable() {
            @Override
            public void run() {
                // open camera for picture taking. save photo to device
                Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                DateFormat dateFormat = new SimpleDateFormat("dd-MM-yy_HH-mm-ss", Locale.getDefault());
                Date date = new Date();
                filename = "ccphoto_" + dateFormat.format(date) + ".png";
                File dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "CritterCap");
                if (!dir.exists()) {
                    dir.mkdirs();
                }
                // carry photo's path on to entry activity
                picture = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES + "/CritterCap/"), filename);
                Uri tempUri = Uri.fromFile(picture);
                intent.putExtra(MediaStore.EXTRA_OUTPUT, tempUri);
                intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0);
                startActivityForResult(intent, 0);
            }
        }, buttonPress.getDuration()*2);

    }

    /* deleteAnimal()
     *
     * this function is triggered when the user presses the "DELETE" button. They are given a chance to confirm
     * their decision, then if confirmed, the animal is removed from the database and the name spinner is reloaded.
     */
    public void deleteAnimal(final View view){
        final Spinner nameSpinner = (Spinner) findViewById(R.id.nameSpinner);
        final Discoveries discoveries = new Discoveries(this, null, null, 1);

        // don't want to crash on empty
        if (nameSpinner == null || nameSpinner.getSelectedItem() == null){
            view.startAnimation(buttonPress);
            // wait for animation to finish... then return
            view.postDelayed(new Runnable() {
                @Override
                public void run() {
                    // do nothing
                }
            }, buttonPress.getDuration()*2);
            return;
        }
        view.startAnimation(buttonPress);
        // wait for animation to finish...
        view.postDelayed(new Runnable() {
            @Override
            public void run() {
                DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        switch (which){
                            case DialogInterface.BUTTON_POSITIVE:
                                // delete animal from db and reload spinner
                                discoveries.deleteAnimal(nameSpinner.getSelectedItem().toString());
                                reloadSpinner();
                                break;

                            case DialogInterface.BUTTON_NEGATIVE:
                                //do nothing
                                break;
                        }
                    }
                };// confirmation prompt
                AlertDialog.Builder builder = new AlertDialog.Builder(view.getContext());
                builder.setMessage(getString(R.string.sure_delete) + nameSpinner.getSelectedItem().toString() + getString(R.string.question_mark))
                        .setPositiveButton("DELETE", dialogClickListener).setNegativeButton("BACK", dialogClickListener).show();
            }
        }, buttonPress.getDuration()*2);
    }

    /* reloadSpinner()
     *
     * this function reloads the name spinner with the database's current entries
     */
    public void reloadSpinner(){
        Spinner nameSpinner = (Spinner) findViewById(R.id.nameSpinner);
        Discoveries discoveries = new Discoveries(this, null, null, 1);

        List<String> animals = discoveries.getAnimals();
        if (!animals.isEmpty()){
            ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, R.layout.spinner_layout, animals);
            dataAdapter.setDropDownViewResource(R.layout.spinner_dropdown_layout);
            nameSpinner.setAdapter(dataAdapter);
        }
    }

    /* popInstructions()
     *
     * this function pops up the instructions prompt, exiting after the user presses anywhere
     */
    public void popInstructions(View view){
        view.startAnimation(buttonPress);
        LinearLayout linLayout = (LinearLayout) findViewById(R.id.linearMain);
        LayoutInflater layoutInflater = (LayoutInflater) getApplicationContext().getSystemService(LAYOUT_INFLATER_SERVICE);
        ViewGroup viewGroup = (ViewGroup) layoutInflater.inflate(R.layout.instructions_layout, null);
        final PopupWindow popup = new PopupWindow(viewGroup, 900, 1000, true);
        popup.showAtLocation(linLayout, Gravity.NO_GRAVITY, 90, 300);
        viewGroup.setOnTouchListener(new View.OnTouchListener(){
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent){
                popup.dismiss();
                return true;
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data){
        // this is used by newAnimal()
        if (requestCode == 0 && resultCode == Activity.RESULT_OK){
            if (picture.exists()) {
                Intent intent = new Intent(this, EntryActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                intent.setFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                intent.putExtra("filename",picture.getAbsolutePath());
                startActivity(intent);
            }
        }
    }

}
