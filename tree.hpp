#pragma once

class Node {
    public:
        Node() = default;
    private:
        unsigned int argc;
        Node *argv[];
};

class Tree {
    public:
        Tree() = default;
    private:
        Node root;
};