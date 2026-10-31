using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using TMPro;
using UnityEngine;
using UnityEngine.InputSystem;

namespace CoinCatcher
{
    // Unity handles input and visuals; the native C++ library owns every game rule.
    public class CoinCatcherGame : MonoBehaviour
    {
        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        public struct GameState
        {
            public int score, bombs;
            public float seconds, basketX;
            public int gameOver, reason;
        }
        [StructLayout(LayoutKind.Sequential, Pack = 4)]
        struct DropState { public int id; public float x, y; public int bomb; }

        const string Library = "CoinCatcherNative";
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)] static extern IntPtr cc_create();
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)] static extern void cc_destroy(IntPtr game);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)] static extern void cc_restart(IntPtr game);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)] static extern void cc_update(IntPtr game, float dt, float direction);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)] static extern void cc_get_state(IntPtr game, out GameState state);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)] static extern int cc_drop_count(IntPtr game);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)] static extern int cc_get_drop(IntPtr game, int index, out DropState drop);

        public RectTransform basket, dropLayer;
        public TextMeshProUGUI scoreLabel, timerLabel, bombsLabel, resultLabel;
        public GameObject gameOverPanel;
        public UnityEngine.UI.Button restartButton;
        public Sprite circle;
        public GameState State { get; private set; }
        public bool NativeConnected => handle != IntPtr.Zero;

        IntPtr handle;
        readonly Dictionary<int, RectTransform> visuals = new Dictionary<int, RectTransform>();
        readonly HashSet<int> visibleIds = new HashSet<int>();
        readonly List<int> removedIds = new List<int>();

        void Awake()
        {
            handle = cc_create();
            if (handle == IntPtr.Zero) throw new InvalidOperationException("Could not create the C++ game.");
            restartButton.onClick.AddListener(RestartGame);
            DrawState();
        }

        void OnDestroy()
        {
            if (restartButton) restartButton.onClick.RemoveListener(RestartGame);
            if (handle != IntPtr.Zero) { cc_destroy(handle); handle = IntPtr.Zero; }
        }

        public void RestartGame()
        {
            if (handle == IntPtr.Zero) return;
            foreach (var visual in visuals.Values) if (visual) Destroy(visual.gameObject);
            visuals.Clear();
            cc_restart(handle);
            DrawState();
        }

        void Update()
        {
            if (handle == IntPtr.Zero) return;
            float direction = 0;
            var keys = Keyboard.current;
            if (keys != null)
            {
                if (keys.aKey.isPressed || keys.leftArrowKey.isPressed) direction -= 1;
                if (keys.dKey.isPressed || keys.rightArrowKey.isPressed) direction += 1;
            }
            cc_update(handle, Time.deltaTime, direction);
            DrawState();
        }

        void DrawState()
        {
            cc_get_state(handle, out GameState state);
            State = state;
            basket.anchoredPosition = new Vector2(state.basketX, -300);
            scoreLabel.text = "Score: " + state.score;
            int seconds = Mathf.CeilToInt(state.seconds);
            timerLabel.text = "Time: " + (seconds / 60).ToString("00") + ":" + (seconds % 60).ToString("00");
            timerLabel.color = state.seconds <= 10 ? new Color32(255, 105, 105, 255) : Color.white;
            bombsLabel.text = "Bombs: " + state.bombs + " / 3";
            gameOverPanel.SetActive(state.gameOver != 0);
            if (state.gameOver != 0)
                resultLabel.text = "Game Over\n<size=25>" + (state.reason == 1 ? "You caught 3 bombs!" : "Time is up!") + "\nScore: " + state.score + "</size>";

            visibleIds.Clear();
            int count = cc_drop_count(handle);
            for (int i = 0; i < count; i++)
            {
                if (cc_get_drop(handle, i, out DropState drop) == 0) continue;
                visibleIds.Add(drop.id);
                if (!visuals.TryGetValue(drop.id, out RectTransform visual))
                {
                    bool bomb = drop.bomb != 0;
                    visual = Shape(bomb ? "Bomb" : "Coin", dropLayer, Vector2.zero, Vector2.one * (bomb ? 34 : 26),
                        bomb ? new Color32(244, 87, 87, 255) : new Color32(255, 204, 64, 255), circle);
                    if (bomb)
                    {
                        Shape("Body", visual, Vector2.zero, Vector2.one * 26, new Color32(32, 35, 46, 255), circle);
                        var fuse = Shape("Fuse", visual, new Vector2(8, 18), new Vector2(5, 14), new Color32(255, 178, 75, 255), null);
                        fuse.localRotation = Quaternion.Euler(0, 0, -30);
                    }
                    visuals.Add(drop.id, visual);
                }
                visual.anchoredPosition = new Vector2(drop.x, drop.y);
            }
            removedIds.Clear();
            foreach (var pair in visuals) if (!visibleIds.Contains(pair.Key)) removedIds.Add(pair.Key);
            foreach (int id in removedIds) { Destroy(visuals[id].gameObject); visuals.Remove(id); }
        }

        static RectTransform Shape(string name, Transform parent, Vector2 position, Vector2 size, Color color, Sprite sprite)
        {
            var go = new GameObject(name, typeof(RectTransform), typeof(UnityEngine.UI.Image));
            var rect = go.GetComponent<RectTransform>();
            rect.SetParent(parent, false); rect.anchoredPosition = position; rect.sizeDelta = size;
            var image = go.GetComponent<UnityEngine.UI.Image>();
            image.color = color; image.sprite = sprite; image.raycastTarget = false;
            return rect;
        }
    }
}
