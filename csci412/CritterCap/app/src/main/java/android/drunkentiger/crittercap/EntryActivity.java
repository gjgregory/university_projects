package android.drunkentiger.crittercap;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;

import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

/*
 * this is the activity where the user provides information to go along with their picture, then either
 * stores it in the database or clicks their device's back button.
 */
public class EntryActivity extends AppCompatActivity {
    String filename;
    Animation buttonPress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_entry);

        buttonPress = AnimationUtils.loadAnimation(this, R.anim.button_press);

        // populate all 3 spinners with options...

        Spinner categorySpinner = (Spinner) findViewById(R.id.categorySpinner);
        // add sizes to spinner list
        List<String> categories = new ArrayList<String>();
        categories.add(getString(R.string.Mammal));
        categories.add(getString(R.string.Fish));
        categories.add(getString(R.string.Bird));
        categories.add(getString(R.string.Reptile));
        categories.add(getString(R.string.Amphibian));
        categories.add(getString(R.string.Insect));
        categories.add(getString(R.string.plant));
        categories.add(getString(R.string.Alien));
        categories.add(getString(R.string.VinDiesel));
        // create adapter for spinner
        ArrayAdapter<String> categoryAdapter = new ArrayAdapter<String>(this, R.layout.spinner_layout_2, categories);
        categoryAdapter.setDropDownViewResource(R.layout.spinner_dropdown_layout);
        categorySpinner.setAdapter(categoryAdapter);

        Spinner colorSpinner = (Spinner) findViewById(R.id.colorSpinner);
        // add colors to spinner list
        List<String> colors = new ArrayList<String>();
        colors.add(getString(R.string.black));
        colors.add(getString(R.string.white));
        colors.add(getString(R.string.brown));
        colors.add(getString(R.string.green));
        colors.add(getString(R.string.red));
        colors.add(getString(R.string.yellow));
        colors.add(getString(R.string.blue));
        colors.add(getString(R.string.orange));
        colors.add(getString(R.string.purple));
        colors.add(getString(R.string.pink));
        colors.add(getString(R.string.multicolor));
        colors.add(getString(R.string.other));
        // create adapter for spinner
        ArrayAdapter<String> colorAdapter = new ArrayAdapter<String>(this, R.layout.spinner_layout_2, colors);
        colorAdapter.setDropDownViewResource(R.layout.spinner_dropdown_layout);
        colorSpinner.setAdapter(colorAdapter);

        Spinner sizeSpinner = (Spinner) findViewById(R.id.sizeSpinner);
        // add sizes to spinner list
        List<String> sizes = new ArrayList<String>();
        sizes.add(getString(R.string.micro));
        sizes.add(getString(R.string.tiny));
        sizes.add(getString(R.string.small));
        sizes.add(getString(R.string.medium));
        sizes.add(getString(R.string.large));
        sizes.add(getString(R.string.huge));
        sizes.add(getString(R.string.giant));
        // create adapter for spinner
        ArrayAdapter<String> sizeAdapter = new ArrayAdapter<String>(this, R.layout.spinner_layout_2, sizes);
        sizeAdapter.setDropDownViewResource(R.layout.spinner_dropdown_layout);
        sizeSpinner.setAdapter(sizeAdapter);

        // get filename from previous activity
        Bundle captureExtras = getIntent().getExtras();
        if (captureExtras != null) {
            filename = captureExtras.getString("filename");
        }
    }

    /*saveEntry()
     *
     * this finalizes the user's discovery and stores it in the database, then returns to the CaptureActivity
     */
    public void saveEntry(View view){
        final Intent intent = new Intent(this, CaptureActivity.class);
        EditText nameField = (EditText)findViewById(R.id.nameField);
        Spinner typeSpinner = (Spinner)findViewById(R.id.categorySpinner);
        Spinner colorSpinner = (Spinner)findViewById(R.id.colorSpinner);
        Spinner sizeSpinner = (Spinner)findViewById(R.id.sizeSpinner);
        EditText traitsField = (EditText)findViewById(R.id.traitsField);

        assert colorSpinner != null;
        assert sizeSpinner != null;
        assert traitsField != null;
        assert typeSpinner != null;
        // create Animal object
        Animal animal = new Animal(nameField != null ? nameField.getText().toString().trim() : null,
                typeSpinner.getSelectedItem().toString(), colorSpinner.getSelectedItem().toString(),
                sizeSpinner.getSelectedItem().toString(), traitsField.getText().toString().trim(), filename);

        Discoveries discoveries = new Discoveries(this, null, null, 1);

        // make sure inputs are valid (spinners are always valid, unique traits are optional)
        if (discoveries.getAnimal(animal.getName()) != null){
            Toast.makeText(this, R.string.name_in_use, Toast.LENGTH_LONG).show();
        }else if (animal.getName().equals("")){
            Toast.makeText(this, R.string.no_name_provided, Toast.LENGTH_LONG).show();
        }else{
            // checks are good. add to DB
            discoveries.addAnimal(animal);
            view.startAnimation(buttonPress);
            view.postDelayed(new Runnable() {
                @Override
                public void run() {
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                    startActivity(intent);
                }
            }, buttonPress.getDuration()*2);
        }

    }
}
