/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * Licensed under the Oculus SDK License Agreement (the "License");
 * you may not use the Oculus SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.
 *
 * You may obtain a copy of the License at
 *
 * https://developer.oculus.com/licenses/oculussdk/
 *
 * Unless required by applicable law or agreed to in writing, the Oculus SDK
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using Oculus.Interaction.Input;
using System;
using UnityEngine;
using UnityEngine.Serialization;

namespace Oculus.Interaction
{
    /// <summary>
    /// Specifies a position and rotation offset from the root of the given hand
    /// </summary>
    public class HandRootOffset : MonoBehaviour
    {
        [SerializeField, Interface(typeof(IHand))]
        private UnityEngine.Object _hand;
        public IHand Hand { get; private set; }

        #region OVR Fields

        [SerializeField]
        [InspectorName("Offset")]
        private Vector3 _offset;

        [SerializeField]
        [InspectorName("Rotation")]
        private Quaternion _rotation = Quaternion.identity;

        #endregion OVR Fields

        #region OpenXR Fields

        [SerializeField]
        [InspectorName("Offset")]
        private Vector3 _posOffset;

        [SerializeField]
        [InspectorName("Rotation")]
        private Quaternion _rotOffset = Quaternion.identity;

        #endregion OpenXR Fields

        [SerializeField]
        [FormerlySerializedAs("_mirrorLeftRotation")]
        [Tooltip("When the attached hand's handedness is set to Left, this property will mirror the offsets. " +
            "This allows for offset values to be set in Right hand coordinates for both Left and Right hands.")]
        private bool _mirrorOffsetsForLeftHand = true;
        public bool MirrorOffsetsForLeftHand
        {
            get => _mirrorOffsetsForLeftHand;
            set => _mirrorOffsetsForLeftHand = value;
        }

        [Header("Freeze rotations")]
        [SerializeField]
        private bool _freezeRotationX = false;
        public bool FreezeRotationX
        {
            get => _freezeRotationX;
            set => _freezeRotationX = value;
        }
        [SerializeField]
        private bool _freezeRotationY = false;
        public bool FreezeRotationY
        {
            get => _freezeRotationY;
            set => _freezeRotationY = value;
        }

        [SerializeField]
        private bool _freezeRotationZ = false;
        public bool FreezeRotationZ
        {
            get => _freezeRotationZ;
            set => _freezeRotationZ = value;
        }

        public Vector3 Offset
        {
#if ISDK_OPENXR_HAND
            get => _posOffset;
            set => _posOffset = value;
#else
            get => _offset;
            set => _offset = value;
#endif
        }

        public Quaternion Rotation
        {
#if ISDK_OPENXR_HAND
            get => _rotOffset;
            set => _rotOffset = value;
#else
            get => _rotation;
            set => _rotation = value;
#endif
        }

        public bool MirrorLeftRotation
        {
            get => _mirrorOffsetsForLeftHand;
            set => _mirrorOffsetsForLeftHand = value;
        }

        private Pose _cachedPose = Pose.identity;

        protected bool _started = false;

        protected virtual void Awake()
        {
            Hand = _hand as IHand;
        }

        protected virtual void Start()
        {
            this.BeginStart(ref _started);
            this.AssertField(Hand, nameof(Hand));
            this.EndStart(ref _started);
        }

        protected virtual void OnEnable()
        {
            if (_started)
            {
                Hand.WhenHandUpdated += HandleHandUpdated;
            }
        }

        protected virtual void OnDisable()
        {
            if (_started)
            {
                Hand.WhenHandUpdated -= HandleHandUpdated;
            }
        }

        private void HandleHandUpdated()
        {
            if (Hand.GetRootPose(out Pose rootPose))
            {
                GetOffset(ref _cachedPose);
                _cachedPose.Postmultiply(rootPose);
                _cachedPose.rotation = FreezeRotation(_cachedPose.rotation);
                transform.SetPose(_cachedPose);
            }
        }

        public void GetOffset(ref Pose pose)
        {
            if (!_started)
            {
                return;
            }

            GetOffset(ref pose, Hand.Handedness, Hand.Scale);
        }

        public void GetOffset(ref Pose pose, Handedness handedness, float scale)
        {
            if (_mirrorOffsetsForLeftHand && handedness == Handedness.Left)
            {
                pose.position = HandMirroring.Mirror(Offset) * scale;
                pose.rotation = HandMirroring.Mirror(Rotation);
#if !ISDK_OPENXR_HAND
                pose.rotation = pose.rotation * Constants.LeftRootRotation;
#endif
            }
            else
            {
                pose.position = Offset * scale;
                pose.rotation = Rotation;
            }
        }

        public void GetWorldPose(ref Pose pose)
        {
            pose.position = this.transform.position;
            pose.rotation = this.transform.rotation;
        }


        private Quaternion FreezeRotation(Quaternion rotation)
        {
            if (_freezeRotationX
                || _freezeRotationY
                || _freezeRotationZ)
            {
                Vector3 eulerAngles = rotation.eulerAngles;
                Quaternion pitch = Quaternion.Euler(new Vector3(eulerAngles.x, 0.0f, 0.0f));
                Quaternion yaw = Quaternion.Euler(new Vector3(0.0f, eulerAngles.y, 0.0f));
                Quaternion roll = Quaternion.Euler(new Vector3(0.0f, 0.0f, eulerAngles.z));
                Quaternion finalSourceRotation = Quaternion.identity;

                if (!_freezeRotationY)
                {
                    finalSourceRotation *= yaw;
                }
                if (!_freezeRotationX)
                {
                    finalSourceRotation *= pitch;
                }
                if (!_freezeRotationZ)
                {
                    finalSourceRotation *= roll;
                }
                rotation = finalSourceRotation;
            }

            return rotation;
        }
        #region Inject
        public void InjectAllHandRootOffset(IHand hand)
        {
            InjectHand(hand);
        }

        public void InjectHand(IHand hand)
        {
            _hand = hand as UnityEngine.Object;
            Hand = hand;
        }

        [Obsolete("Use the " + nameof(Offset) + " setter instead")]
        public void InjectOffset(Vector3 offset)
        {
            Offset = offset;
        }

        [Obsolete("Use the " + nameof(Rotation) + " setter instead")]
        public void InjectRotation(Quaternion rotation)
        {
            Rotation = rotation;
        }

        [Obsolete("Use " + nameof(InjectAllHandRootOffset) + " instead")]
        public void InjectAllHandWristOffset(IHand hand,
            Vector3 offset, Quaternion rotation)
        {
            InjectHand(hand);
            InjectOffset(offset);
            InjectRotation(rotation);
        }

        #endregion
    }
}
