/**
 * Created by lishuhuakai on 16-7-28.
 */
import java.util.HashMap;
import java.util.Vector;

public class AttrTab {
    private HashMap<String, Vector<attr>> cls_attrs;
    public AttrTab() {
        /* 默认的构造函数 */
        cls_attrs = new HashMap<>();
    }

    public void addClsAttr(String clsName, attr atr) {
        /* 用于向类中添加属性 */
        Vector<attr> attrs = cls_attrs.get(clsName);
        if (attrs == null) {
            attrs = new Vector<>();
            attrs.add(atr); /* 在末尾添加一个属性atr */
            cls_attrs.put(clsName, attrs);
        } else {
            attrs.add(atr);
        }
    }

    public int getClsAttrOffset(String clsName, String attrName) {
        /* 用于获取某个类的属性的偏移地址*/
        Vector<attr> vct = cls_attrs.get(clsName);
        if (vct != null) {
            for (int i = vct.size() - 1; i >= 0; i--) {
                if (vct.get(i).name.str.equals(attrName))
                    return i;
            }
        }
        return -1;
    }
}
