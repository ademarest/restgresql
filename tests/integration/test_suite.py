import requests
import sys

BASE_URL = "http://localhost:8000"

def test_all_posts():
    print("Testing /api/posts...", end=" ")
    response = requests.get(f"{BASE_URL}/api/posts")
    assert response.status_code == 200, f"Expected 200, got {response.status_code}"
    data = response.json()
    assert isinstance(data, list), "Expected a list of posts"
    assert len(data) > 0, "Expected at least one post"
    print("PASS")

def test_post_by_id():
    print("Testing /api/posts/1...", end=" ")
    response = requests.get(f"{BASE_URL}/api/posts/1")
    assert response.status_code == 200, f"Expected 200, got {response.status_code}"
    data = response.json()
    assert data["postTitle"] == "Test Article", f"Expected 'Test Article', got {data.get('postTitle')}"
    print("PASS")

def test_recent_posts():
    print("Testing /api/recentPosts/1...", end=" ")
    response = requests.get(f"{BASE_URL}/api/recentPosts/1")
    assert response.status_code == 200, f"Expected 200, got {response.status_code}"
    data = response.json()
    assert isinstance(data, list), "Expected a list of posts"
    assert len(data) == 1, f"Expected 1 post, got {len(data)}"
    print("PASS")

def test_image_by_id():
    print("Testing /api/images/1...", end=" ")
    response = requests.get(f"{BASE_URL}/api/images/1")
    assert response.status_code == 200, f"Expected 200, got {response.status_code}"
    assert len(response.content) > 0, "Expected image content, but got empty response"
    print("PASS")

def test_image_by_filename():
    print("Testing /api/images/test.png...", end=" ")
    response = requests.get(f"{BASE_URL}/api/images/test.png")
    assert response.status_code == 200, f"Expected 200, got {response.status_code}"
    assert len(response.content) > 0, "Expected image content, but got empty response"
    print("PASS")

def test_404():
    print("Testing 404...", end=" ")
    response = requests.get(f"{BASE_URL}/api/nonexistent")
    assert response.status_code == 404, f"Expected 404, got {response.status_code}"
    print("PASS")

if __name__ == "__main__":
    try:
        test_all_posts()
        test_post_by_id()
        test_recent_posts()
        test_image_by_id()
        test_image_by_filename()
        test_404()
        print("\nAll tests passed successfully!")
    except Exception as e:
        print(f"\nTEST FAILED: {e}")
        sys.exit(1)
