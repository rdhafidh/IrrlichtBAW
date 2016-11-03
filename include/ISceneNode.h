// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_SCENE_NODE_H_INCLUDED__
#define __I_SCENE_NODE_H_INCLUDED__

#include "ESceneNodeTypes.h"
#include "ECullingTypes.h"
#include "EDebugSceneTypes.h"
#include "ISceneNodeAnimator.h"
#include "SMaterial.h"
#include "irrString.h"
#include "aabbox3d.h"
#include "matrix4.h"
#include "irrList.h"
#include "IOcclusionQuery.h"
#include "IDummyTransformationSceneNode.h"

namespace irr
{
namespace scene
{
	class ISceneManager;
    class ISceneNode;



	//! Scene node interface.
	/** A scene node is a node in the hierarchical scene graph. Every scene
	node may have children, which are also scene nodes. Children move
	relative to their parent's position. If the parent of a node is not
	visible, its children won't be visible either. In this way, it is for
	example easily possible to attach a light to a moving car, or to place
	a walking character on a moving platform on a moving ship.
	*/
	class ISceneNode : public IDummyTransformationSceneNode
	{
	public:

		//! Constructor
		ISceneNode(IDummyTransformationSceneNode* parent, ISceneManager* mgr, s32 id=-1,
				const core::vector3df& position = core::vector3df(0,0,0),
				const core::vector3df& rotation = core::vector3df(0,0,0),
				const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f))
			:   IDummyTransformationSceneNode(parent,position,rotation,scale),
                SceneManager(mgr), query(0), ID(id), AutomaticCullingState(EAC_FRUSTUM_BOX),
                DebugDataVisible(EDS_OFF), mobid(0), mobtype(0), IsVisible(true),
                IsDebugObject(false), staticmeshid(0),blockposX(0),blockposY(0),blockposZ(0), renderPriority(0x80000000u)
		{
		}


		//! Destructor
		virtual ~ISceneNode()
		{
            if (query)
                query->drop();
		}

        virtual const bool isISceneNode() const {return true;}


		//! This method is called just before the rendering process of the whole scene.
		/** Nodes may register themselves in the render pipeline during this call,
		precalculate the geometry which should be renderered, and prevent their
		children from being able to register themselves if they are clipped by simply
		not calling their OnRegisterSceneNode method.
		If you are implementing your own scene node, you should overwrite this method
		with an implementation code looking like this:
		\code
		if (IsVisible)
			SceneManager->registerNodeForRendering(this);

		ISceneNode::OnRegisterSceneNode();
		\endcode
		*/
		virtual void OnRegisterSceneNode()
		{
			OnRegisterSceneNode_static(this);
		}


		virtual void setOcclusionQuery(video::IOcclusionQuery* query_in)
		{
		    video::IOcclusionQuery* old_query = query;

            query = query_in;

            if (query)
                query->grab();

            if (old_query)
                old_query->drop();
		}

		virtual video::IOcclusionQuery* getOcclusionQuery() {return query;}
		virtual video::IOcclusionQuery const* getOcclusionQuery() const {return query;}


		//! Adds a child to this scene node.
		/** If the scene node already has a parent it is first removed
		from the other parent.
		\param child A pointer to the new child. */
		virtual void addChild(ISceneNode* child)
		{
			if (child && (child != this))
			{
				// Change scene manager?
				if (SceneManager != child->SceneManager)
					child->setSceneManager(SceneManager);
			}

			IDummyTransformationSceneNode::addChild(child);
		}

		//! OnAnimate() is called just before rendering the whole scene.
		/** Nodes may calculate or store animations here, and may do other useful things,
		depending on what they are. Also, OnAnimate() should be called for all
		child scene nodes here. This method will be called once per frame, independent
		of whether the scene node is visible or not.
		\param timeMs Current time in milliseconds. */
		virtual void OnAnimate(u32 timeMs)
		{
			OnAnimate_static(this,timeMs);
		}


		//! Renders the node.
		virtual void render() = 0;


		//! Returns the name of the node.
		/** \return Name as character string. */
		virtual const c8* getName() const
		{
			return Name.c_str();
		}


		//! Sets the name of the node.
		/** \param name New name of the scene node. */
		virtual void setName(const c8* name)
		{
			Name = name;
		}


		//! Sets the name of the node.
		/** \param name New name of the scene node. */
		virtual void setName(const core::stringc& name)
		{
			Name = name;
		}


		//! Get the axis aligned, not transformed bounding box of this node.
		/** This means that if this node is an animated 3d character,
		moving in a room, the bounding box will always be around the
		origin. To get the box in real world coordinates, just
		transform it with the matrix you receive with
		getAbsoluteTransformation() or simply use
		getTransformedBoundingBox(), which does the same.
		\return The non-transformed bounding box. */
		virtual const core::aabbox3d<f32>& getBoundingBox() = 0;


		//! Get the axis aligned, transformed and animated absolute bounding box of this node.
		/** \return The transformed bounding box. */
		virtual const core::aabbox3d<f32> getTransformedBoundingBox()
		{
			core::aabbox3d<f32> box = getBoundingBox();
			AbsoluteTransformation.transformBoxEx(box);
			return box;
		}

		inline const uint32_t& getRenderPriorityScore() const {return renderPriority;}

		inline void setRenderPriorityScore(const uint32_t& nice) {renderPriority = nice;}


		//! Returns whether the node should be visible (if all of its parents are visible).
		/** This is only an option set by the user, but has nothing to
		do with geometry culling
		\return The requested visibility of the node, true means
		visible (if all parents are also visible). */
		virtual bool isVisible() const
		{
			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
			return IsVisible;
		}

		//! Check whether the node is truly visible, taking into accounts its parents' visibility
		/** \return true if the node and all its parents are visible,
		false if this or any parent node is invisible. */
		virtual bool isTrulyVisible() const
		{
			return isTrulyVisible_static(this);
		}

		//! Sets if the node should be visible or not.
		/** All children of this node won't be visible either, when set
		to false. Invisible nodes are not valid candidates for selection by
		collision manager bounding box methods.
		\param isVisible If the node shall be visible. */
		virtual void setVisible(bool isVisible_in)
		{
			IsVisible = isVisible_in;
		}


		//! Get the id of the scene node.
		/** This id can be used to identify the node.
		\return The id. */
		virtual s32 getID() const
		{
			return ID;
		}


		//! Sets the id of the scene node.
		/** This id can be used to identify the node.
		\param id The new id. */
		virtual void setID(s32 id)
		{
			ID = id;
		}


		//! Returns the material based on the zero based index i.
		/** To get the amount of materials used by this scene node, use
		getMaterialCount(). This function is needed for inserting the
		node into the scene hierarchy at an optimal position for
		minimizing renderstate changes, but can also be used to
		directly modify the material of a scene node.
		\param num Zero based index. The maximal value is getMaterialCount() - 1.
		\return The material at that index. */
		virtual video::SMaterial& getMaterial(u32 num)
		{
			return video::IdentityMaterial;
		}


		//! Get amount of materials used by this scene node.
		/** \return Current amount of materials of this scene node. */
		virtual u32 getMaterialCount() const
		{
			return 0;
		}


		//! Sets all material flags at once to a new value.
		/** Useful, for example, if you want the whole mesh to be
		affected by light.
		\param flag Which flag of all materials to be set.
		\param newvalue New value of that flag. */
		void setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue)
		{
			for (u32 i=0; i<getMaterialCount(); ++i)
				getMaterial(i).setFlag(flag, newvalue);
		}


		//! Sets the texture of the specified layer in all materials of this scene node to the new texture.
		/** \param textureLayer Layer of texture to be set. Must be a
		value smaller than MATERIAL_MAX_TEXTURES.
		\param texture New texture to be used. */
		void setMaterialTexture(u32 textureLayer, video::ITexture* texture)
		{
			if (textureLayer >= video::MATERIAL_MAX_TEXTURES)
				return;

			for (u32 i=0; i<getMaterialCount(); ++i)
				getMaterial(i).setTexture(textureLayer, texture);
		}


		//! Sets the material type of all materials in this scene node to a new material type.
		/** \param newType New type of material to be set. */
		void setMaterialType(video::E_MATERIAL_TYPE newType)
		{
			for (u32 i=0; i<getMaterialCount(); ++i)
				getMaterial(i).MaterialType = newType;
		}


		//! Enables or disables automatic culling based on the bounding box.
		/** Automatic culling is enabled by default. Note that not
		all SceneNodes support culling and that some nodes always cull
		their geometry because it is their only reason for existence.
		\param state The culling state to be used. */
		void setAutomaticCulling( u32 state)
		{
			AutomaticCullingState = state;
		}


		//! Gets the automatic culling state.
		/** \return The automatic culling state. */
		u32 getAutomaticCulling() const
		{
			return AutomaticCullingState;
		}


		//! Sets if debug data like bounding boxes should be drawn.
		/** A bitwise OR of the types from @ref irr::scene::E_DEBUG_SCENE_TYPE.
		Please note that not all scene nodes support all debug data types.
		\param state The debug data visibility state to be used. */
		virtual void setDebugDataVisible(u32 state)
		{
			DebugDataVisible = state;
		}

		//! Returns if debug data like bounding boxes are drawn.
		/** \return A bitwise OR of the debug data values from
		@ref irr::scene::E_DEBUG_SCENE_TYPE that are currently visible. */
		u32 isDebugDataVisible() const
		{
			return DebugDataVisible;
		}


		//! Sets if this scene node is a debug object.
		/** Debug objects have some special properties, for example they can be easily
		excluded from collision detection or from serialization, etc. */
		void setIsDebugObject(bool debugObject)
		{
			IsDebugObject = debugObject;
		}


		//! Returns if this scene node is a debug object.
		/** Debug objects have some special properties, for example they can be easily
		excluded from collision detection or from serialization, etc.
		\return If this node is a debug object, true is returned. */
		bool isDebugObject() const
		{
			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
			return IsDebugObject;
		}


		//! Returns type of the scene node
		/** \return The type of this node. */
		virtual ESCENE_NODE_TYPE getType() const
		{
			return ESNT_UNKNOWN;
		}


		//! Creates a clone of this scene node and its children.
		/** \param newParent An optional new parent.
		\param newManager An optional new scene manager.
		\return The newly created clone of this node. */
		virtual IDummyTransformationSceneNode* clone(IDummyTransformationSceneNode* newParent=0, ISceneManager* newManager=0)
		{
			return 0; // to be implemented by derived classes
		}

		//! Retrieve the scene manager for this node.
		/** \return The node's scene manager. */
		virtual ISceneManager* getSceneManager(void) const { return SceneManager; }

		// sodan
		int32_t mobtype;
		int32_t mobid;
		int32_t staticmeshid;
		int32_t blockposX,blockposY,blockposZ;

	protected:

		//! A clone function for the ISceneNode members.
		/** This method can be used by clone() implementations of
		derived classes
		\param toCopyFrom The node from which the values are copied
		\param newManager The new scene manager. */
		virtual void cloneMembers(ISceneNode* toCopyFrom, ISceneManager* newManager)
		{
			Name = toCopyFrom->Name;
			ID = toCopyFrom->ID;
			AutomaticCullingState = toCopyFrom->AutomaticCullingState;
			DebugDataVisible = toCopyFrom->DebugDataVisible;
			IsVisible = toCopyFrom->IsVisible;
			IsDebugObject = toCopyFrom->IsDebugObject;
			if (newManager)
				SceneManager = newManager;
			else
				SceneManager = toCopyFrom->SceneManager;

            IDummyTransformationSceneNode::cloneMembers(toCopyFrom,SceneManager);
		}

		//! Sets the new scene manager for this node and all children.
		//! Called by addChild when moving nodes between scene managers
		void setSceneManager(ISceneManager* newManager)
		{
			setSceneManager_static(this,newManager);
		}

		//! Name of the scene node.
		core::stringc Name;

		//! Pointer to the scene manager
		ISceneManager* SceneManager;

		//! Pointer to the attached occlusion query
		video::IOcclusionQuery* query;

		//! ID of the node.
		s32 ID;

		//! Automatic culling state
		u32 AutomaticCullingState;

		//! Flag if debug data should be drawn, such as Bounding Boxes.
		u32 DebugDataVisible;

		//! Is the node visible?
		bool IsVisible;

		uint32_t renderPriority;

		//! Is debug object?
		bool IsDebugObject;

        static void OnAnimate_static(IDummyTransformationSceneNode* node, u32 timeMs)
		{
            ISceneNode* tmp = static_cast<ISceneNode*>(node);
			if (!node->isISceneNode()||tmp->IsVisible)
			{
				// animate this node with all animators

				ISceneNodeAnimatorList::ConstIterator ait = node->getAnimators().begin();
				while (ait != node->getAnimators().end())
                {
					// continue to the next node before calling animateNode()
					// so that the animator may remove itself from the scene
					// node without the iterator becoming invalid
					ISceneNodeAnimator* anim = *ait;
					++ait;
					anim->animateNode(node, timeMs);
				}

				// update absolute position
				node->updateAbsolutePosition();

				// perform the post render process on all children

				IDummyTransformationSceneNodeList::ConstIterator it = node->getChildren().begin();
				for (; it != node->getChildren().end(); ++it)
                {
                    ISceneNode* tmpChild = static_cast<ISceneNode*>(*it);
                    if ((*it)->isISceneNode())
                        static_cast<ISceneNode*>(*it)->OnAnimate(timeMs);
                    else
                        OnAnimate_static(*it,timeMs);
                }
			}
		}
    private:
        static void OnRegisterSceneNode_static(IDummyTransformationSceneNode* node)
        {
            ISceneNode* tmp = static_cast<ISceneNode*>(node);
			if (!node->isISceneNode()||tmp->IsVisible)
			{
				IDummyTransformationSceneNodeList::ConstIterator it = node->getChildren().begin();
				for (; it != node->getChildren().end(); ++it)
                {
                    if ((*it)->isISceneNode())
                        static_cast<ISceneNode*>(*it)->OnRegisterSceneNode();
                    else
                        OnRegisterSceneNode_static(*it);
                }
			}
        }

        static bool isTrulyVisible_static(const IDummyTransformationSceneNode* node)
		{
			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
            const ISceneNode* tmp = static_cast<const ISceneNode*>(node);

            if (node->isISceneNode())
            {
                if(!tmp->isVisible())
                    return false;
            }

            if (node->getParent())
                return isTrulyVisible_static(node->getParent());
            else
                return true;
		}


		static void setSceneManager_static(IDummyTransformationSceneNode* node, ISceneManager* newManager)
		{
            ISceneNode* tmp = static_cast<ISceneNode*>(node);

            if (node->isISceneNode())
                tmp->SceneManager = newManager;

			IDummyTransformationSceneNodeList::ConstIterator it = node->getChildren().begin();
			for (; it != node->getChildren().end(); ++it)
                setSceneManager_static(*it,newManager);
		}
	};


} // end namespace scene
} // end namespace irr

#endif
