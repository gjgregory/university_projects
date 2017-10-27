package android.drunkentiger.crittercap;

/**
 * Created by garrett on 6/1/16.
 *
 * this is the DB Handler class
 */
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import java.util.ArrayList;
import java.util.List;



public class Discoveries extends SQLiteOpenHelper {
    private Context context;
    private static final int DATABASE_VERSION = 1;
    private static final String DATABASE_NAME = "discoveriesDB.db";
    private static final String TABLE_ANIMALS = "animals";

    public static final String COLUMN_NAME = "name";
    public static final String COLUMN_CATEGORY = "category";
    public static final String COLUMN_COLOR = "color";
    public static final String COLUMN_SIZE = "size";
    public static final String COLUMN_TRAITS = "traits";
    public static final String COLUMN_FILENAME = "filename";



    public Discoveries(Context context, String name, SQLiteDatabase.CursorFactory factory, int version) {
        super(context, DATABASE_NAME, factory, DATABASE_VERSION);
        this.context = context;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        String CREATE_ANIMALS_TABLE = "CREATE TABLE " + TABLE_ANIMALS + "(" + COLUMN_NAME +
                " TEXT PRIMARY KEY, " + COLUMN_CATEGORY + " TEXT," + COLUMN_COLOR + " TEXT," + COLUMN_SIZE + " TEXT," +
                COLUMN_TRAITS + " TEXT," + COLUMN_FILENAME + " TEXT" + ")";
        db.execSQL(CREATE_ANIMALS_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion,
                          int newVersion) {
    }

    /*addAnimal()
     *
     * this stores a new row in the animals table
     */
    public void addAnimal(Animal animal) {

        ContentValues values = new ContentValues();
        values.put(COLUMN_NAME, animal.getName());
        values.put(COLUMN_CATEGORY, animal.getCategory());
        values.put(COLUMN_COLOR, animal.getColor());
        values.put(COLUMN_SIZE, animal.getSize());
        values.put(COLUMN_TRAITS, animal.getTraits());
        values.put(COLUMN_FILENAME, animal.getFilename());


        SQLiteDatabase db = this.getWritableDatabase();

        db.insert(TABLE_ANIMALS, null, values);
        db.close();
    }

    /*getAnimal()
     *
     * this returns an animal object containing cell information from the row specified by String name
     */
    public Animal getAnimal(String name) {
        String query = "Select * FROM " + TABLE_ANIMALS + " WHERE " + COLUMN_NAME + " =  \"" + name + "\"";

        SQLiteDatabase db = this.getReadableDatabase();

        Cursor cursor = db.rawQuery(query, null);
        cursor.moveToFirst();

        if (cursor.isAfterLast()){
            return null;
        }

        Animal animal = new Animal(cursor.getString(0), cursor.getString(1), cursor.getString(2),
                cursor.getString(3), cursor.getString(4), cursor.getString(5));

        cursor.close();
        db.close();

        return animal;
    }

    /*deleteAnimal()
     *
     * this removes a row from the animals table corresponding to String name
     */
    public void deleteAnimal(String name) {
        String query = "Select * FROM " + TABLE_ANIMALS + " WHERE " + COLUMN_NAME + " =  \"" + name + "\"";

        SQLiteDatabase db = this.getWritableDatabase();

        db.delete(TABLE_ANIMALS, COLUMN_NAME + " = ?", new String[]{name});
        db.close();
    }

    /*getAnimals()
     *
     * this returns an ArrayList of Strings that match the names of all rows in the animals table. It is
     * used to update the name spinner in the CaptureActivity.
     */
    public List<String> getAnimals(){
        ArrayList<String> animals = new ArrayList<>();

        String query = "Select * FROM " + TABLE_ANIMALS;
        SQLiteDatabase db = this.getReadableDatabase();//
        //db.execSQL("delete from "+ TABLE_ANIMALS);

        Cursor cursor = db.rawQuery(query, null);
        cursor.moveToFirst();
        while (!cursor.isAfterLast()){
            animals.add(cursor.getString(0));
            cursor.moveToNext();
        }
        cursor.close();
        db.close();

        return animals;
    }

}