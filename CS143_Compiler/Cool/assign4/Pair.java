/**
 * Created by lishuhuakai on 16-7-29.
 */
public class Pair {
    private String first;
    private String second;
    public String getFirst() {
        return first;
    }
    public String getSecond() {
        return second;
    }
    public void setFirst(String first) {
        this.first = first;
    }
    public Pair(String first, String second) {
        this.first = first;
        this.second = second;
    }
    public String toString() {
        return getFirst() + "." + getSecond();
    }
}
