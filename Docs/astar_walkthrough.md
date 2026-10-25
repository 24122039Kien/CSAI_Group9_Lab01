# Giải thích và kiểm tra phần A*

Tài liệu đi cùng `ai/solvers/astar_solver.py`, dùng để đọc code, chuẩn bị
review và viết phần mô tả thuật toán trong báo cáo. Thuật toán tìm kiếm đã
chạy được với `zero_heuristic`. Manhattan là phần tích hợp với TV5.

## 1. A* đang giải bài toán gì?

Mỗi node là một `State(x, y, orientation, bridge_mask)`. Vị trí, tư thế và
cấu hình cầu đều ảnh hưởng tới các nước đi hợp lệ. Cùng tọa độ nhưng khác
`bridge_mask` phải được xét là hai trạng thái khác nhau.

Solver lấy trạng thái đầu từ `problem.initial_state()`, lấy các nước hợp lệ
bằng `problem.successors(state)` và kiểm tra thắng bằng `problem.is_goal(state)`.
Luật lăn khối, ô dễ vỡ, cầu và công tắc được mô phỏng trong `problem.py`.

## 2. Ba giá trị g, h, f

| Giá trị | Ý nghĩa |
|---|---|
| `g(state)` | Chi phí của đường đã tìm được từ điểm đầu tới `state` |
| `h(state)` | Ước lượng chi phí còn lại từ `state` tới đích |
| `f(state) = g(state) + h(state)` | Ưu tiên khi chọn trạng thái để xét tiếp |

Ví dụ minh họa: A có `g = 4, h = 3` nên `f = 7`; B có `g = 2, h = 6` nên
`f = 8`. A* chọn A trước. Khi `h = 0` cho mọi trạng thái, A* ưu tiên theo
`g` và hoạt động như UCS. Với chi phí 1 mỗi nước, tổng chi phí cũng là số bước.

## 3. Các cấu trúc dữ liệu

| Biến | Vai trò |
|---|---|
| `frontier` | Heap chứa các bản ghi đang chờ được xét |
| `best_g[state]` | Chi phí tốt nhất đã tìm được tới trạng thái đó |
| `parent[state]` | Cặp `(trạng thái cha, hành động)` để truy vết |
| `counter` | Số thứ tự duy nhất cho mỗi bản ghi đưa vào heap |

Mỗi bản ghi heap có dạng `(f, h, counter, g, state)`. Python so sánh lần lượt
các thành phần: ưu tiên `f` nhỏ, sau đó `h` nhỏ và thứ tự thêm vào. `counter`
giúp tránh việc Python phải so sánh trực tiếp hai đối tượng `State` bằng điểm.
Giá trị `g` được lưu riêng để nhận ra bản ghi thuộc một đường cũ.

## 4. Đọc hàm solve theo từng bước

1. Khởi tạo `best_g[start] = 0`, `parent[start] = None` rồi đưa trạng thái
   đầu vào heap với `f = 0 + h(start)`.
2. Lấy bản ghi có ưu tiên cao nhất bằng `heappop`.
3. Nếu `g` trong bản ghi khác `best_g[state]`, đã có đường rẻ hơn tới cùng
   trạng thái. Bỏ qua bản ghi cũ này.
4. Nếu trạng thái là đích, truy vết `parent` về điểm đầu, đảo thứ tự các
   hành động và trả kết quả qua `make_result`.
5. Với mỗi con `(action, next_state, step_cost)`, tính `new_g = g + step_cost`.
   Chỉ khi `new_g` tốt hơn giá trị đã lưu mới cập nhật `best_g`, `parent`
   và đưa một bản ghi mới vào heap.
6. Nếu heap hết mà chưa tìm được đích, trả `make_result(None)`.

Ví dụ, trước đó tới B tốn 6, sau này có đường mới chỉ tốn 4. Ta đổi
`best_g[B]` thành 4 và đổi cha của B theo đường mới. Bản ghi có `g = 6`
vẫn có thể còn trong heap, nhưng sẽ được bỏ qua ở bước 3. Cách này gọi là
lazy deletion, tránh phải tìm và xóa phần tử giữa heap.

## 5. Các quyết định cần giải thích khi review

**Kiểm tra đích khi lấy khỏi heap.** Lần đầu sinh ra trạng thái đích có thể
đi qua một đường đắt. Trong test, đường thẳng tới đích tốn 10 nhưng đi qua
một trạng thái trung gian chỉ tốn 2. Trả ngay khi sinh đích sẽ chọn sai.

**Cho phép mở rộng lại trạng thái.** `best_g` có thể tiếp tục giảm sau khi
trạng thái đã được xét. Cài đặt này cho phép mở rộng lại khi có đường tốt hơn,
hỗ trợ heuristic admissible nhưng không consistent. Nếu khóa trạng thái bằng
một tập visited vĩnh viễn, có thể mất đường tối ưu khi heuristic không consistent.

**Điều kiện tối ưu.** Với chi phí mỗi cạnh dương, heuristic admissible
(`h(s)` không vượt chi phí thật tới đích), `h(goal) = 0`, và cho phép mở rộng
lại khi tìm được `g` tốt hơn, A* trả về lời giải tối ưu. Trước khi một đích
đắt hơn tối ưu có thể được lấy ra, trên một đường tối ưu vẫn có node với
`f` không vượt chi phí tối ưu, nên node đó phải được ưu tiên trước.
Consistency là điều kiện mạnh hơn: `h(s) <= cost(s, a, s') + h(s')`.

**Truy vết.** `parent[B] = (A, "R")` nghĩa là từ A đi R để đến B. Đi ngược
từ đích qua các cha rồi đảo danh sách sẽ tạo chuỗi hành động để phát lại.

## 6. Các chỉ số kết quả

- `nodes_expanded`: số bản ghi còn hiệu lực được lấy ra rồi sinh con;
  không tính bản ghi cũ hoặc trạng thái đích.
- `nodes_generated`: số trạng thái con hợp lệ đã sinh ra, kể cả con không
  cải thiện `best_g`.
- `max_frontier`: số bản ghi trong heap lớn nhất, gồm cả bản ghi cũ chưa bị lấy ra.
- `BaseSolver.run()` đo thời gian và có thể đo bộ nhớ; `make_result()` tính
  chi phí của chuỗi hành động trả về.

## 7. Quy trình tích hợp với TV5

Giao diện heuristic là `h(problem, state) -> float`. Khi TV5 hoàn thiện
`manhattan_heuristic` trong `ai/heuristics.py`, chạy từ thư mục gốc repository:

```bash
python -m pytest tests/test_astar_solver.py tests/test_astar_integration.py -q -rs
python main.py code/data/1.data --algo astar --heuristic zero
python main.py code/data/1.data --algo astar --heuristic manhattan
python main.py code/data/2.data --algo astar --heuristic manhattan
```

Kết quả tối ưu với chi phí 1: màn 1 là 10 bước, màn 2 là 68 bước. So sánh thêm
node mở rộng và thời gian của hai heuristic; ghi nhận số đo thực tế.
Khi TV3 bổ sung chi phí không đồng nhất, cần kiểm tra chi phí A* bằng UCS
với cùng hàm chi phí và heuristic được hiệu chỉnh theo hàm chi phí đó.

## 8. Bộ test tích hợp kiểm tra những gì?

`tests/test_astar_integration.py` dựng toàn bộ đồ thị trạng thái đạt được từ
điểm đầu trên từng màn mẫu, coi trạng thái thắng là kết thúc. Sau đó BFS
trên các cạnh ngược từ mọi đích tính số bước tối thiểu còn lại. Việc đảo
đúng các cạnh đã quan sát giữ được tính có hướng do công tắc và cầu.

Các test đối chiếu heuristic với chi phí thật trên mọi trạng thái có đường
tới đích, kiểm tra `h(goal) = 0`, consistency trên mọi cạnh, rồi đối chiếu
lời giải A* với chuẩn BFS độc lập. Trạng thái không thể tới đích không có
cận trên hữu hạn để kiểm tra admissibility, nhưng các cạnh của nó vẫn được
kiểm tra consistency. Kiểm thử hai màn này bổ sung bằng chứng thực nghiệm;
phần chứng minh heuristic vẫn cần bao quát các màn và luật được hỗ trợ.

Nhánh `zero` chạy ngay; nhánh `manhattan` báo SKIP rõ nguyên nhân khi hàm
còn `NotImplementedError`, rồi tự chạy khi TV5 bổ sung cài đặt.

## 9. Chuẩn bị phần báo cáo và vấn đáp

Mô tả trạng thái, hàm đánh giá `f`, vòng lặp, điều kiện tối ưu và các chỉ số
thực nghiệm. Với không gian hữu hạn, bộ nhớ phụ thuộc số trạng thái đã lưu
và số bản ghi heap; thời gian phụ thuộc số lần sinh con, mở rộng lại và thao
tác heap. Các đồ thị nhỏ dùng trong test còn kiểm tra chi phí không đồng
nhất, đường tốt hơn, trạng thái đầu là đích và màn vô nghiệm.

Trước khi trình bày, tự giải thích bằng một ví dụ vì sao cần `bridge_mask`,
`counter`, `best_g`, lazy deletion, kiểm tra đích khi pop và `parent`.
Ghi công cụ AI và prompt đã sử dụng theo quy định phụ lục của nhóm.
