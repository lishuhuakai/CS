from JS import graphics

def interpret(trees): # Hello, Friend
    for tree in trees: # Hello
        # ("word-element", "Hello,")
        nodetype = tree[0] # "word-element"
        if nodetype == "word-element":
            graphics.word(tree[1])
        elif nodetype == "tag-element":
            # <b>Strong text!</b>
            tagname = tree[1] # b
            tagargs = tree[2] # []
            subtrees = tree[3] # ...Strong text!...
            closetagname = tree[4] # b
            # QUIZ: (1) check that the tags match!
            # if not, use graphics.warning()
            # (2) interpret the subtree!
            if tagname != closetagname:
                graphics.warning("not equal")
            graphics.begintag(tagname, tagargs)
            interpret(subtrees)
            graphics.endtag()

