/**
 * Created by lishuhuakai on 16-7-29.
 */
import java.util.Enumeration;
import java.util.Vector;
import java.util.HashMap;

public class DispTab {
    /**
     * 这个类主要是用于自己来实现disptTab
     */
    private HashMap<String, Vector<Pair>> dispTab;

    public DispTab () {
        dispTab = new HashMap<>();
    }

    public void addClsMethod(String selfName, String baseName, String mtdName) {
        Vector<Pair> pairs = dispTab.get(selfName);
        if (pairs == null) {
            pairs = new Vector<>();
            pairs.add(new Pair(baseName, mtdName));
            dispTab.put(selfName, pairs);
        } else {
            boolean flag = true;
            for (int i = 0; i < pairs.size(); ++i) {
                Pair pair = pairs.get(i);
                if (pair.getSecond().equals(mtdName)) {
                    pair.setFirst(baseName);
                    flag = false;
                    break;
                }
            }
            if (flag) pairs.add(new Pair(baseName, mtdName));
        }
    }

    public int getClsMtdOffset(String clsName, String mtdName) {
        Vector<Pair> pairs = dispTab.get(clsName);
        if (pairs != null) {
            for (int i = 0; i < pairs.size(); ++i) {
                Pair pair = pairs.get(i);
                if (pair.getSecond().equals(mtdName))
                    return i;
            }
        }
        return -1;
    }

    public Vector<Pair> getTab(String name) {
        return dispTab.get(name);
    }

    public static void main(String[] args) {
        DispTab dispTab = new DispTab();

    }
}
