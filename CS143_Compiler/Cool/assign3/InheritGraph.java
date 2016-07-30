/**
 * Created by lishuhuakai on 16-7-18.
 */

import java.util.*;
import java.util.Map.Entry;

class Node {
    /* 构造一个内部类*/
    public String type;
    public Node parent; /* 一般来说,只有一个父节点吧! */
    public HashSet<Node> children; /* 但是可能存在一堆的子节点 */

    public Node getparent() {

        return parent;
    }

    public boolean equal(Node another) {
            /* 用于比较两个Node是否相等 */
        return this.type.equals(another.type); /* 只比较两者的类型! */
    }

    public Node(String type, Node parent, HashSet<Node> children) { /* 构造函数! */
        this.type = type;
        this.parent = parent;
        this.children = children;
    }

    public Node(String type, Node parent) { /* 构造函数! */
        this.type = type;
        this.parent = parent;
        this.children = new HashSet<>();
    }

    public boolean equals(Object obj) {
        if (obj instanceof Node) {
            Node nd = (Node) obj;
            if (nd.type.equals(this.type))
                return true;
        }
        return false;
    }
    public int hashCode()
    {
        return type.hashCode();
    }

    public void addChild(Node cd) {
        children.add(cd);
    }
    public String toString() {
        return type;
    }
}

public class InheritGraph {
    /* 好吧,自己来实现所谓的继承图 */
    public HashMap<String, Node> hmap;

    public boolean addEdge(String type, String parent) {
        /* 一般而言,我们是不知道子节点的! */
        Node parentVal = hmap.get(parent); /* 找到父节点 */
        Node childVal = hmap.get(type);
        if (childVal == null && parentVal != null) {
            Node nd = new Node(type, parentVal);
            parentVal.addChild(nd); /* 添加子节点 */
            //System.out.println("Parent --> " + parentVal.type);
            //System.out.println("Child --> " + nd.type);
            hmap.put(type, nd);
            return true;
        }
        else if (childVal != null && parentVal != null) {
            // 子节点不为空
            childVal.parent = parentVal;
            parentVal.addChild(childVal);
            return true;
        }
        return false;
    }
    public void printGraph() {
        /* 现在要输出对应的图 */
        for (Map.Entry<String, Node> entry : hmap.entrySet()) {
            String res = "Key = " + entry.getKey() + ", Parent = ";
            Node nd = entry.getValue().parent;
            if (nd == null)
                res += " ";
            else
                res += nd.type;
            System.out.println(res);
        }
    }

    public void rprintGraph() {
        // reverse print
        for (Map.Entry<String, Node> entry : hmap.entrySet()) {
            String res = "Key = " + entry.getKey() + ", child = ";
            Iterator<Node> it = entry.getValue().children.iterator();
            while(it.hasNext()) {
                res += it.next().type + " ";
            }
            System.out.println(res);
        }
    }

    public Node getNode(String key) {
        return hmap.get(key);
    }

    private boolean isRoot(Node root, Node child) {
        /* 用于判断是否为root节点*/
        //System.out.println("---------------isRoot---------------");
        //System.out.println("root --> " + root.type);
        //System.out.println("type --> " + child.type);
        if (root.type.equals(child.type))
            return true;
        Iterator<Node> it = root.children.iterator();
        while (it.hasNext()) {
            Node val = it.next();
            //System.out.println(val.type);
            if (isRoot(val, child))
                return true;
        }
        return false;
    }

    private String findRoot(Node l, Node r) {
       // System.out.println("--------------findRoot-------------");
        //System.out.println("l --> " + l.type);
        //System.out.println("r --> " + r.type);
        if ((l == null) || (r == null))
            return null;

        if (l.type.equals(r.type))
            return l.type;
        else if (isRoot(l, r)) /* 如果左边的是右边的root节点也行 */
            return l.type;
        else
            return findRoot(l.parent, r);
        /* 应该来说,一般都可以找到,因为所有节点都有一个共同的祖先root */
    }

    public String lub(String l, String r) {
        /* 找到两者的共同祖先 */
        //System.out.println("---------------lub----------------");
        //System.out.println("l --> " + l);
        //System.out.println("r --> " + r);
        Node nl = hmap.get(l);
        Node nr = hmap.get(r);
        return findRoot(nl, nr);
    }

    public InheritGraph() {
        hmap = new HashMap<>();
        Node nd = new Node("Object", null);
        hmap.put("Object", nd); /* 首先添加一个Object节点 */
    }

    public boolean isDescendentOf(String tpa, String tpb) {
        /* 用于判断tpa是不是tpb的子类! */
        //System.out.println(" -> " + lub(tpa, tpb));
        //System.out.println(tpa);
        //System.out.println(tpb);
        // 如果是子类的话,那么它们两者的lua的值应该等于tpa
        // 因为一个很重要的原因,

        //String res = lub(tpa, tpb);
        //printGraph();
        return lub(tpa, tpb).equals(tpb);
    }
    public static void main(String[] args) {
        InheritGraph hg = new InheritGraph();
        hg.addEdge("Int", "Object");

        hg.addEdge("Bool", "Object");
        hg.addEdge("Main", "Object");
        hg.addEdge("IO", "Object");
        hg.addEdge("Bazz", "IO");
        hg.addEdge("Foo", "Bazz");
        hg.addEdge("Razz", "Foo");
        hg.addEdge("Bar", "Razz");
        hg.addEdge("String", "Object");
        hg.printGraph();
        System.out.println(hg.lub("Bar", "Bar"));
        System.out.println(hg.lub("Bar", "Razz"));
    }
}
