/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef _BTRIE_H
#define _BTRIE_H

#include "Base.h"

namespace NVR
{

template<typename T, typename K> class BTrieNode
{
public:
    typedef K KeyType;
    typedef T* NodePtr;

    enum
    {
        KeyBucketCount = sizeof(KeyType) * 8,
        InitialMaskShift = sizeof(KeyType) * 8 - 1
    };

    BTrieNode(void)
            : mKey(0)
    {
    }

    BTrieNode(KeyType key)
            : mKey(key)
    {
    }

    ~BTrieNode(void)
    {
    }

    FORCE_INLINE KeyType key(void) const
    {
        return mKey;
    }

    FORCE_INLINE KeyType &key(void)
    {
        return mKey;
    }

    FORCE_INLINE NodePtr child(int num) const
    {
        return mChild[num];
    }

    FORCE_INLINE NodePtr &child(int num)
    {
        return mChild[num];
    }

    FORCE_INLINE static int GetKeyBucketIndex(KeyType key)
    {
        return key == 0 ? (KeyBucketCount - 1) : __builtin_clz(key);
    }

protected:
    KeyType mKey;
    NodePtr mChild[2];

private:
    BTrieNode(BTrieNode const &);
    BTrieNode &operator=(BTrieNode const &);
};

template<typename T> class BTrieSimpleUIntNode
{
public:
    typedef uint KeyType;
    typedef T* NodePtr;

    enum
    {
        KeyBucketCount = 1,
        InitialMaskShift = 31
    };

    BTrieSimpleUIntNode(void)
            : mKey(0)
    {
    }

    BTrieSimpleUIntNode(KeyType key)
            : mKey(key)
    {
    }

    ~BTrieSimpleUIntNode(void)
    {
    }

    FORCE_INLINE KeyType key(void) const
    {
        return mKey;
    }

    FORCE_INLINE KeyType &key(void)
    {
        return mKey;
    }

    FORCE_INLINE NodePtr child(int num) const
    {
        return mChild[num];
    }

    FORCE_INLINE NodePtr &child(int num)
    {
        return mChild[num];
    }

    FORCE_INLINE static int GetKeyBucketIndex(KeyType key)
    {
        (void)key;
        return 0;
    }

protected:
    KeyType mKey;
    NodePtr mChild[2];

private:
    BTrieSimpleUIntNode(BTrieSimpleUIntNode const &);
    BTrieSimpleUIntNode &operator=(BTrieSimpleUIntNode const &);
};

template<typename T> class BTrie
{
public:
    typedef typename T::NodePtr NodePtr;
    typedef typename T::KeyType KeyType;

    BTrie(void)
    {
        for (int i = 0; i < T::KeyBucketCount; i++)
        {
            mBuckets[i] = 0;
        }
    }

    NodePtr insert(NodePtr node)
    {
        KeyType nodeKey = node->key();
        int bindex = T::GetKeyBucketIndex(nodeKey);
        KeyType mask = static_cast<KeyType>(1) << (T::InitialMaskShift - bindex);
        NodePtr *currentNodePtr = &mBuckets[bindex];
        NodePtr currentNode = *currentNodePtr;

        while (currentNode != 0)
        {
            if (currentNode->key() == nodeKey)
            {
                return currentNode;
            }
            mask >>= 1;
            currentNodePtr = &currentNode->child((nodeKey & mask) != 0);
            currentNode = *currentNodePtr;
        }

        *currentNodePtr = node;
        node->child(0) = 0;
        node->child(1) = 0;

        return node;
    }

    NodePtr remove(KeyType key)
    {
        int bindex = T::GetKeyBucketIndex(key);
        KeyType mask = static_cast<KeyType>(1) << (T::InitialMaskShift - bindex);
        NodePtr *currentNodePtr = &mBuckets[bindex];
        NodePtr currentNode = *currentNodePtr;

        while (currentNode != 0)
        {
            if (currentNode->key() == key)
            {
                break;
            }
            mask >>= 1;
            currentNodePtr = &currentNode->child((key & mask) != 0);
            currentNode = *currentNodePtr;
        }

        if (currentNode == 0)
        {
            return 0;
        }

        // replace removed node with any leaf node
        NodePtr *leafNodePtr = currentNodePtr;
        NodePtr leafNode = currentNode;
        for (;;)
        {
            // TODO: investigate how child removal priority affects trie quality
            if (leafNode->child(0) != 0)
            {
                leafNodePtr = &leafNode->child(0);
                leafNode = *leafNodePtr;
            }
            else if (leafNode->child(1) != 0)
            {
                leafNodePtr = &leafNode->child(1);
                leafNode = *leafNodePtr;
            }
            else
            {
                break;
            }
        }

        *leafNodePtr = 0;
        if (leafNode != currentNode)
        {
            // attach leaf node to the tree only if its not the node we remove
            *currentNodePtr = leafNode;
            leafNode->child(0) = currentNode->child(0);
            leafNode->child(1) = currentNode->child(1);
        }

        return currentNode;
    }

    NodePtr get(KeyType key) const
    {
        int bindex = T::GetKeyBucketIndex(key);
        KeyType mask = static_cast<KeyType>(1) << (T::InitialMaskShift - bindex);
        NodePtr currentNode = mBuckets[bindex];

        while (currentNode != 0)
        {
            if (currentNode->key() == key)
            {
                return currentNode;
            }
            mask >>= 1;
            currentNode = currentNode->child((key & mask) != 0);
        }

        return 0;
    }

    bool empty(void) const
    {
        for (int i = 0; i < T::KeyBucketCount; i++)
        {
            if (mBuckets[i] != 0)
            {
                return false;
            }
        }

        return true;
    }

    NodePtr getMinimum(void) const
    {
        NodePtr currentNode;
        for (int i = T::KeyBucketCount - 1; i >= 0; i--)
        {
            currentNode = mBuckets[i];
            if (currentNode != 0)
            {
                break;
            }
        }

        if (currentNode == 0)
        {
            return 0;
        }

        KeyType candidateKey = currentNode->key();
        NodePtr candidateNode = currentNode;
        currentNode = currentNode->child(currentNode->child(0) == 0);

        while (currentNode != 0)
        {
            KeyType currentKey = currentNode->key();
            if (currentKey < candidateKey)
            {
                candidateKey = currentKey;
                candidateNode = currentNode;
            }

            currentNode = currentNode->child(currentNode->child(0) == 0);
        }

        return candidateNode;
    }

    NodePtr getMaximum(void) const
    {
        NodePtr currentNode;
        for (int i = 0; i < T::KeyBucketCount; i++)
        {
            currentNode = mBuckets[i];
            if (currentNode != 0)
            {
                break;
            }
        }

        if (currentNode == 0)
        {
            return 0;
        }

        KeyType candidateKey = currentNode->key();
        NodePtr candidateNode = currentNode;
        currentNode = currentNode->child(currentNode->child(1) == 0);

        while (currentNode != 0)
        {
            KeyType currentKey = currentNode->key();
            if (currentKey > candidateKey)
            {
                candidateKey = currentKey;
                candidateNode = currentNode;
            }

            currentNode = currentNode->child(currentNode->child(1) == 0);
        }

        return candidateNode;
    }

private:
    NodePtr mBuckets[T::KeyBucketCount];
};

}

#endif

