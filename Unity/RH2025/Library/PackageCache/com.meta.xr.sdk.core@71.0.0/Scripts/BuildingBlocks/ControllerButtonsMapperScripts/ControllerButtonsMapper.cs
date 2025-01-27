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

using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

namespace Meta.XR.BuildingBlocks
{
    /// <summary>
    /// A utility class for mapping controller buttons easily.
    /// </summary>
    /// <example>
    /// <code>
    /// // Instantiate a new button action
    /// var buttonAction = new ControllerButtonsMapper.ButtonClickAction
    /// {
    ///     Title = "Spawn Object",
    ///     Button = OVRInput.Button.PrimaryIndexTrigger,
    ///     ButtonMode = ControllerButtonsMapper.ButtonClickAction.ButtonClickMode.OnButtonUp,
    ///     Callback = new UnityEvent()
    /// };
    /// </code>
    /// </example>
    public class ControllerButtonsMapper : MonoBehaviour
    {
        /// <summary>
        /// A struct to consolidate all the options for a button action.
        /// </summary>
        /// <example>
        /// <code>
        /// // Instantiate a new button action
        /// var buttonAction = new ControllerButtonsMapper.ButtonClickAction
        /// {
        ///     Title = "Spawn Object",
        ///     Button = OVRInput.Button.PrimaryIndexTrigger,
        ///     ButtonMode = ControllerButtonsMapper.ButtonClickAction.ButtonClickMode.OnButtonUp,
        ///     Callback = new UnityEvent()
        /// };
        /// </code>
        /// </example>
        [Serializable]
        public struct ButtonClickAction
        {
            /// <summary>
            /// Button click mode types.
            /// </summary>
            /// <remarks>
            /// OnButtonDown will trigger on the first frame when the button is down.
            /// OnButtonUp will trigger on the first frame when the user presses releases the button.
            /// OnButton triggers repeatedly when the user holds the button down.
            /// </remarks>
            public enum ButtonClickMode
            {
                OnButtonUp,
                OnButtonDown,
                OnButton
            }

            /// <summary>
            /// A title for this button action.
            /// </summary>
            public string Title;

            /// <summary>
            /// Sets the button that will trigger the <see cref="Callback"/> when the <see cref="ButtonMode"/> is detected (usually ButtonMode.OnButtonUp).
            /// </summary>
            public OVRInput.Button Button;

            /// <summary>
            /// Button click type: OnButtonUp, OnButtonDown, and OnButton. Use OnButtonUp to trigger the callback when the user releases the button.
            /// </summary>
            public ButtonClickMode ButtonMode;

            /// <summary>
            /// Dispatches when <see cref="Button"/> matches the chosen <see cref="ButtonMode"/>.
            /// </summary>
            public UnityEvent Callback;
        }

        [SerializeField]
        private List<ButtonClickAction> _buttonClickActions;

        /// <summary>
        /// A list of <see cref="ButtonClickAction"/> to trigger.
        /// </summary>
        public List<ButtonClickAction> ButtonClickActions
        {
            get => _buttonClickActions;
            set => _buttonClickActions = value;
        }

        private void Update()
        {
            foreach (var buttonClickAction in ButtonClickActions)
            {
                ButtonClickAction.ButtonClickMode buttonMode = buttonClickAction.ButtonMode;
                OVRInput.Button button = buttonClickAction.Button;

                if ((buttonMode == ButtonClickAction.ButtonClickMode.OnButtonUp && OVRInput.GetUp(button)) ||
                    (buttonMode == ButtonClickAction.ButtonClickMode.OnButtonDown && OVRInput.GetDown(button)) ||
                    (buttonMode == ButtonClickAction.ButtonClickMode.OnButton && OVRInput.Get(button)))
                {
                    buttonClickAction.Callback?.Invoke();
                }
            }
        }
    }
}
