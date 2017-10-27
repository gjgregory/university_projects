package android.drunkentiger.crittercap;

/**
 * Created by garrett on 6/1/16.
 *
 * this is the Animal class which contains all information of a single row in the Animals table in the DB
 */
public class Animal {
    private String name;
    private String category;
    private String color;
    private String size;
    private String traits;
    private String filename;

    public Animal(String name, String category, String color, String size, String traits, String filename) {
        this.name = name;
        this.category = category;
        this.color = color;
        this.size = size;
        this.traits = traits;
        this.filename = filename;
    }
    public String getName(){
        return name;
    }
    public String getCategory(){
        return category;
    }
    public String getColor(){
        return color;
    }
    public String getSize(){
        return size;
    }
    public String getTraits(){
        return traits;
    }
    public String getFilename(){
        return filename;
    }
}